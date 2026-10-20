#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t low = (uint32_t)challenge;
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t a = 5 * high ^ (low + 0x2e0c);
    uint32_t b = (mid ^ 0x45d8) + 2 * low;

    values[0] = 3 * (b - a) + 0xbbef;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 28) & 0x7f) ^ 0x1b;

    for (uint32_t i = 1; i < 6; ++i) {
        unsigned shift = 10 + 5 * (i - 1);

        uint32_t part =
            (uint32_t)((challenge >> shift) & 0x3f);

        values[i] = (3U * values[i - 1]) ^ (part + i);
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t selector = (uint32_t)(challenge & 7);

    uint32_t x =
        (uint32_t)(challenge >> 32) ^
        (uint32_t)(challenge >> 3);

    values[0] = selector;

    switch (selector) {
        case 0:
            values[1] = (x ^ 0xdfecU) + (x >> 2);
            break;

        case 1:
            values[1] = 5U * x + 0x4e6e7U;
            break;

        case 2:
            values[1] = 5U * x + 0xaa95U;
            break;

        case 3:
            values[1] = (2U * x) ^ 0x2147U;
            break;

        case 4:
            values[1] = 5U * x + 0xf541U;
            break;

        case 5:
            values[1] = 5U * x + 0x20dU;
            break;

        case 6:
            values[1] = (4U * x) ^ 0x2a09U;
            break;

        case 7:
            values[1] = (x ^ 0x5f5cU) + (x >> 2);
            break;

        default:
            return 1;
    }

    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk_solver(
    uint32_t target,
    uint32_t low,
    uint32_t high,
    uint32_t extra
) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + 0x72U) ^ extra;
    }

    uint32_t result;

    if (target < mid) {
        result = p4_walk_solver(target, low, mid - 1, extra);
        return (3U * result) ^ (mid + extra) ^ 0xaU;
    } else {
        result = p4_walk_solver(target, mid + 1, high, extra);
        return (3U * result) ^ (mid + extra) ^ 0x1eU;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)(challenge & 0x7f);

    uint32_t extra =
        (uint32_t)(challenge >> 7) ^
        (uint32_t)(challenge >> 32);

    values[0] = p4_walk_solver(target, 0, 127, extra);

    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    // 按链表遍历顺序排列的节点数据
    static const uint32_t node_value[11] = {
        0x633, 0x86b, 0x030, 0x467,
        0x369, 0x96d, 0x276, 0xa16,
        0x51d, 0xb73, 0x147
    };

    static const uint32_t node_salt[11] = {
        0xad2d, 0xb9a9, 0xeb52, 0x826d,
        0xf097, 0xe91c, 0x48b4, 0x7073,
        0x6cf0, 0x1634, 0x77e4
    };

    uint32_t start = (uint32_t)(challenge & 7);
    uint32_t steps = (uint32_t)((challenge >> 3) & 3);

    uint32_t acc =
        (uint32_t)(challenge >> 6) ^
        (uint32_t)(challenge >> 32) ^
        0xe178U;

    for (uint32_t i = start; i <= start + steps; ++i) {
        acc = (5U * (acc + node_salt[i])) ^ node_value[i];
    }

    values[0] = acc;

    values[1] = node_value[start + steps];

    return write_answer(out, cap, values, count);
}

static uint32_t p6_apply(uint32_t op, uint32_t x) {
    switch (op) {
        case 0:
            return (x + 0xe222U) ^ (x >> 5);

        case 1:
            return (x >> 1) | (x << 31);

        case 2:
            return ((x << 5) + 0x84c5U) ^ (3U * x);

        case 3:
            return (x ^ 0x9e32U) + (x << 7);

        case 4:
            return ((x >> 5) ^ x) + 0x760fU;

        case 5:
            return ((x << 2) | (x >> 30)) ^ 0xcbfU;

        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    // 函数指针表的实际顺序
    static const uint32_t ops[8] = {
        5, 3, 0, 0, 3, 1, 4, 2
    };

    // 从 challenge 提取初始参数
    uint32_t x =
        (uint32_t)(challenge >> 6) ^
        (uint32_t)(challenge >> 32);

    // 两次函数调用的索引
    uint32_t i = (uint32_t)(challenge & 7);
    uint32_t j = (uint32_t)((challenge >> 3) & 7);

    // 第一次间接调用
    uint32_t first = p6_apply(ops[i], x);

    // 第二次间接调用
    uint32_t second = p6_apply(ops[j], first ^ 0x23e3U);

    // 最终结果
    values[0] = second + 0xbeaU;

    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    return 1;
}
