/**
 * @file template.hpp
 * @author Yufei li, Jingxuan Liu
 * @brief AI Template for easy use
 * @date 2023-04-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "control.hpp"
#include "simulate.hpp"

#include <vector>
#include <functional>

/**
 * @brief Callback that takes player id and game info as input and returns operations.
*/
using AI = std::function<std::vector<Operation>(int, const GameInfo &)>;


/**
 * @brief Run the game with an AI that depends only on player id and game state.
 * @param ai AI callback.
 */
static void run_with_ai(AI ai)
{
    Controller c;
    while (true)
    {
        if (c.self_player_id == 0) // Game process when you are player 0
        {
            // AI makes decisions
            std::vector<Operation> ops = ai(c.self_player_id, c.get_info());

            // Add operations to controller
            for (auto &op : ops)
                c.append_self_operation(op);

            // Send operations to judger
            c.send_self_operations();

            // Apply operations to game state
            c.apply_self_operations();

            // Read opponent operations from judger
            c.read_opponent_operations();

            // Apply opponent operations to game state
            c.apply_opponent_operations();

            // Read round info from judger
            c.read_round_info();
        }
        else // Game process when you are player 1
        {
            // Read opponent operations from judger
            c.read_opponent_operations();

            // Apply opponent operations to game state
            c.apply_opponent_operations();

            // AI makes decisions
            std::vector<Operation> ops = ai(c.self_player_id, c.get_info());

            // Add operations to controller
            for (auto &op : ops)
                c.append_self_operation(op);

            // Send operations to judger
            c.send_self_operations();

            // Apply operations to game state
            c.apply_self_operations();

            // Read round info from judger
            c.read_round_info();
        }
    }
}