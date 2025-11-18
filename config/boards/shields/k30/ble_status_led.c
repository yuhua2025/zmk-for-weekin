/*
 * Dayu k30 蓝牙指示灯控制
 * 功能：简单可靠的单色LED指示灯控制
 * 注意：此文件使用单色LED配置，通过gpio1引脚10控制
 */

// 条件编译，处理标准C环境和Zephyr环境
#ifdef __ZEPHYR__
// 在Zephyr环境中使用真实的API
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// 注册日志记录器
LOG_MODULE_REGISTER(ble_status_led, LOG_LEVEL_INF);

// Zephyr特定的GPIO标志
#define LED_FLAGS GPIO_OUTPUT_ACTIVE
#else
// 在标准C环境中提供模拟实现
#include <stddef.h>
#include <stdio.h>

// 模拟设备结构体
typedef struct { int dummy; } device;

// 定义必要的常量
#define GPIO_OUTPUT_ACTIVE 0x0001
#define GPIO_OUTPUT 0x0001
#define APPLICATION 0

// 简单的日志宏定义
#define LOG_ERR(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#define LOG_INF(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_DBG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

// 模拟延时函数
static void k_msleep(int ms) {
    (void)ms; // 避免未使用参数警告
    // 在标准C环境中不做实际延时
}

// 模拟Zephyr API函数
static const device *device_get_binding(const char *port_name) {
    printf("模拟获取GPIO设备: %s\n", port_name);
    static device dummy_dev;
    return &dummy_dev;
}

static int device_is_ready(const device *dev) {
    return (dev != NULL);
}

static int gpio_pin_configure(const device *dev, unsigned int pin, int flags) {
    printf("模拟配置GPIO引脚 %u, 标志: %x\n", pin, flags);
    (void)dev; // 避免未使用参数警告
    return 0;
}

static int gpio_pin_set(const device *dev, unsigned int pin, int value) {
    printf("模拟设置GPIO引脚 %u 状态: %d\n", pin, value);
    (void)dev; // 避免未使用参数警告
    return 0;
}

// 模拟SYS_INIT宏
#define SYS_INIT(func, level, prio) \
    static int (*const __init_##func)(void) = &func;

// 模拟CONFIG_APPLICATION_INIT_PRIORITY
#define CONFIG_APPLICATION_INIT_PRIORITY 90

// 模拟Zephyr的LED_FLAGS
#define LED_FLAGS GPIO_OUTPUT_ACTIVE

// 在TEST_BUILD模式下添加main函数用于测试
#ifdef TEST_BUILD
int main(void) {
    printf("\n=== BLE状态LED测试程序 ===\n");
    printf("注意：这是模拟环境，不会控制真实硬件\n\n");
    
    // 调用模块初始化
    int ret = ble_led_module_init();
    
    if (ret == 0) {
        printf("\n测试成功完成！\n");
    } else {
        printf("\n测试失败，返回码: %d\n", ret);
    }
    
    return ret;
}
#endif
#endif

// LED硬件配置 - 使用k30.overlay中定义的GPIO1和PIN 10
#define LED_PORT "gpio1"
#define LED_PIN 10

// 全局变量用于保存GPIO设备
static const device *led_dev = NULL;


/**
 * @brief 初始化蓝牙状态LED
 * 
 * 配置并打开LED指示灯，用于显示蓝牙状态
 * 
 * @return 成功返回0，失败返回错误码
 */
static int ble_status_led_init(void) {
    int ret;
    
    // 获取GPIO设备
    led_dev = device_get_binding(LED_PORT);
    if (led_dev == NULL) {
        LOG_ERR("无法获取GPIO设备: %s", LED_PORT);
        return -1; // 返回错误，设备未找到
    }
    
    // 确保设备已准备就绪
    if (!device_is_ready(led_dev)) {
        LOG_ERR("GPIO设备未准备好: %s", LED_PORT);
        return -2; // 返回错误，设备未准备好
    }
    
    // 配置LED引脚为输出模式
#ifdef __ZEPHYR__
    // Zephyr环境中使用正确的参数
    ret = gpio_pin_configure(led_dev, LED_PIN, LED_FLAGS);
#else
    // 标准C环境中使用模拟实现
    ret = gpio_pin_configure(led_dev, LED_PIN, GPIO_OUTPUT);
#endif
    
    if (ret != 0) {
        LOG_ERR("配置LED引脚失败: %d", ret);
        return -3; // 返回错误，配置失败
    }
    
    // 配置成功，打开LED进行测试
    ret = gpio_pin_set(led_dev, LED_PIN, 1);
    if (ret != 0) {
        LOG_ERR("设置LED状态失败: %d", ret);
        return -4; // 返回错误，设置LED状态失败
    }
    
    LOG_INF("蓝牙状态LED初始化成功");
    return 0;
}

/**
 * @brief 闪烁LED灯
 * 
 * 控制LED灯闪烁指定次数
 * 
 * @param blink_count 闪烁次数
 * @param delay_ms 每次闪烁的延迟时间(毫秒)
 * @return 成功返回0，失败返回错误码
 */
static int blink_led(int blink_count, int delay_ms) {
    // 检查设备是否已初始化
    if (led_dev == NULL || !device_is_ready(led_dev)) {
        LOG_ERR("LED设备未初始化，无法闪烁");
        return -1; // 设备未初始化或未准备好
    }
    
    LOG_INF("LED闪烁测试: %d次，每次延时%dms", blink_count, delay_ms);
    
    // 实现闪烁逻辑
    for (int i = 0; i < blink_count; i++) {
        gpio_pin_set(led_dev, LED_PIN, 0); // 关闭LED
#ifdef __ZEPHYR__
        k_msleep(delay_ms);
#else
        k_msleep(delay_ms); // 在标准C环境中使用模拟的延时函数
#endif
        gpio_pin_set(led_dev, LED_PIN, 1); // 打开LED
#ifdef __ZEPHYR__
        k_msleep(delay_ms);
#else
        k_msleep(delay_ms); // 在标准C环境中使用模拟的延时函数
#endif
    }
    
    return 0;
}

/**
 * @brief 设置LED状态
 * 
 * 控制LED灯的开关状态
 * 
 * @param state 0表示关闭，1表示打开
 * @return 成功返回0，失败返回错误码
 */
int ble_status_led_set(int state) {
    // 检查设备是否已初始化
    if (led_dev == NULL || !device_is_ready(led_dev)) {
        LOG_ERR("LED设备未初始化，无法设置状态");
        return -1; // 设备未初始化
    }
    
    LOG_DBG("设置LED状态: %d", state);
    // 设置LED状态
    int ret = gpio_pin_set(led_dev, LED_PIN, state);
    if (ret != 0) {
        LOG_ERR("设置LED状态失败: %d", ret);
    }
    return ret;
}

/**
 * @brief 模块初始化入口点
 * 
 * 提供手动初始化LED的接口
 * 
 * @return 成功返回0，失败返回错误码
 */
int ble_led_module_init(void) {
    int ret = ble_status_led_init();
    
    // 初始化成功后尝试闪烁LED，用于测试
    if (ret == 0) {
        blink_led(3, 100); // 闪烁3次，每次延时100ms
        LOG_INF("LED模块初始化完成并通过测试");
    } else {
        LOG_ERR("LED模块初始化失败: %d", ret);
    }
    
    return ret;
}

// 使用Zephyr的SYS_INIT宏注册初始化函数
#ifdef __ZEPHYR__
// Zephyr环境中使用真实的优先级
SYS_INIT(ble_status_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
#else
// 标准C环境中的模拟实现已经在前面定义
SYS_INIT(ble_status_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
#endif