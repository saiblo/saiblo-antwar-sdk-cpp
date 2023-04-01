/**
 * @file io.hpp
 * @author Yufei Li, Jingxuan Liu
 * @brief A module for communicating with Judger.
 * @date 2023-04-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include "common.hpp"

/* Input */

using InitInfo = std::pair<int, unsigned long long>;

/** 
 * @brief Read information for initialization.
 * @return Your player ID and the seed for random number generator, together in a pair.
 */
inline InitInfo read_init_info()
{
    int self_player_id;
    unsigned long long seed;
    std::cin >> self_player_id >> seed;
    return {self_player_id, seed};
}

/**
 * @brief Read your opponent's operations and deserialize them. The time to call this
 * function depends on your player ID.
 * @return A vector of Operation objects.
 */
inline std::vector<Operation> read_opponent_operations()
{
    std::vector<Operation> ops;
    int count, type, arg0, arg1 = -1;
    std::cin >> count;
    ops.reserve(count);
    for (int i = 0; i < count; i++)
    {
        std::cin >> type;
        if (type == UpgradeGeneratedAnt || type == UpgradeGenerationSpeed)
        {
            ops.emplace_back(static_cast<OperationType>(type));   
        }
        else if (type == DowngradeTower)
        {
            std::cin >> arg0;
            ops.emplace_back(static_cast<OperationType>(type), arg0);
        }
        else
        {
            std::cin >> arg0 >> arg1;
            ops.emplace_back(static_cast<OperationType>(type), arg0, arg1);
        }
    }
    return ops;
}

/**
 * @brief A combination of deserialized information about current round state received from judger.
 */ 
struct RoundInfo
{
    int round;
    std::vector<Tower> towers;
    std::vector<Ant> ants;
    int coin0, coin1, hp0, hp1;
};

/**
 * @brief Read information at the beginning of a round and deserialize.
 * @return A RoundInfo object with everything received and deserialized.
 */
inline RoundInfo read_round_info()
{
    RoundInfo info;
    // Round ID
    std::cin >> info.round;
    // Variables
    int id, player, x, y, type, cd, hp, level, age, state;
    // Tower
    int tower_num;
    std::cin >> tower_num;
    info.towers.reserve(tower_num);
    for (int i = 0; i < tower_num; ++i)
    {
        std::cin >> id >> player >> x >> y >> type >> cd;
        info.towers.emplace_back(id, player, x, y, static_cast<TowerType>(type), cd);
    }
    // Ant
    int ant_num;
    std::cin >> ant_num;
    info.ants.reserve(ant_num);
    for (int i = 0; i < ant_num; ++i)
    {
        std::cin >> id >> player >> x >> y >> hp >> level >> age >> state;
        info.ants.emplace_back(id, player, x, y, hp, level, age, static_cast<AntState>(state));
    }
    // Coin
    std::cin >> info.coin0 >> info.coin1;
    // Base hp
    std::cin >> info.hp0 >> info.hp1;

    return info;
}

/* Output helpers */

/**
 * @brief Calculate the length of the serialized result of a non-negative integer,
 *        without actually serializing it (i.e. no string is ever constructed). Here we
 *        convert the integer to a string of its decimal representation to serialize it.
 * @param x The non-negative integer to serialize.
 * @return Number of bytes of the result.
 */
inline std::size_t object_length(int x)
{
    std::size_t len = 0;
    do {
        ++len;
        x /= 10;
    } while (x);
    return len;
}

/**
 * @brief Calculate the length of the serialized result of a string of ASCII characters,
 *        which is just the length of the string, of course.
 * @param str The string to serialize.
 * @return The number of bytes of the result.
 */
inline std::size_t object_length(const std::string& str)
{
    return str.length();
}

/**
 * @brief Calculate the length of the serialized result of an Operation object, without
 *        actually serializing it (i.e. no string is ever constructed). The serialized
 *        result includes the type and arguments of the operation, the spaces between
 *        these integers and the trailing line break.
 * @param op The Operation object to serialize.
 * @return Number of bytes of the result.
 */
inline std::size_t object_length(const Operation& op)
{
    std::size_t len = object_length(op.type);
    if (op.arg0 != Operation::INVALID_ARG)
        len += 1 + object_length(op.arg0);
    if (op.arg1 != Operation::INVALID_ARG)
        len += 1 + object_length(op.arg1);
    len += 1; // Length of line break
    return len;
}

/**
 * @brief Calculate the length of the serialized result of some Operation objects,
 *        without actually serializing them (i.e. no string is ever constructed).
 *        the serialized result is simply the sum of each serialized Operation object.
 * @param ops A vector of Operation objects to serialize.
 * @return Number of bytes of the result.
 */
inline std::size_t object_length(const std::vector<Operation>& ops)
{
    std::size_t len = 0;
    for (auto& op: ops)
        len += object_length(op);
    return len;
}

/**
 * @brief Convert an object into big-endian representation.
 * @param src Pointer to the memory of the object to be converted.
 * @param size Size of the object in bytes.
 * @param dest Buffer area for converted result.
 */
inline void convert_to_big_endian(const void* src, std::size_t size, void* dest)
{
    for (std::size_t i = 0; i < size; ++i)
        static_cast<char*>(dest)[size - i - 1] = static_cast<const char*>(src)[i];
}

/* Output */

/**
 * @brief Print the header, i.e. the total size in big-endian binary representation.
 * @param size The total size of everything to be sent.
 */
inline void print_header(int size)
{
    // Convert into big-endian order
    char buf[4] = {};
    convert_to_big_endian(&size, sizeof(size), buf);
    for (int i = 0; i < 4; ++i)
        std::cout << buf[i];
}

/**
 * @brief Send raw string with header to judger.
 * @param str String to be sent.
 */
inline void send_string(const std::string& str)
{
    print_header(object_length(str));
    std::cout << str;
}

/**
 * @brief Send some serialized operations with header to judger.
 * @param ops A vector of Operation objects to be sent.
 */
inline void send_operations(const std::vector<Operation>& ops)
{
    // Get the total length, including the leading operation num
    std::size_t op_len = object_length(ops);
    std::size_t op_num_len = object_length(ops.size()) + 1;
    int total_len = static_cast<int>(op_num_len + op_len);
    // Print the header
    print_header(total_len);
    // Print the content
    std::cout << ops.size() << std::endl;
    for (auto &op : ops)
    {
        std::cout << op; // There has been a line break for each operation
    }
}
