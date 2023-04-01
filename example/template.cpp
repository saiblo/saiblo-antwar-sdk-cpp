#include "../include/template.hpp"

// A simple AI that always try building towers
std::vector<Operation> simple_ai(int player_id, const GameInfo &game_info)
{
    std::vector<Operation> ops; // Operations to be returned

    if (player_id == 0) // Try building towers at (5, 9), (5, 3), (5, 15) for player 0
    {
        ops.emplace_back(BuildTower, 5, 9);
        ops.emplace_back(BuildTower, 5, 3);
        ops.emplace_back(BuildTower, 5, 15);
    }
    else // Try building towers at (13, 9), (13, 3), (13, 15) for player 1
    {
        ops.emplace_back(BuildTower, 13, 9);
        ops.emplace_back(BuildTower, 13, 3);
        ops.emplace_back(BuildTower, 13, 15);
    }

    return ops;
}

// An advanced AI using a simulator for decision
std::vector<Operation> advanced_ai(int player_id, const GameInfo &game_info)
{
    Simulator s(game_info); // Create a simulator

    // Simulate 10 rounds
    for (int i = 0; i < 10; ++i)
    {
        Operation build_tower0(OperationType::BuildTower, 5, 9);
        Operation build_tower1(OperationType::BuildTower, 13, 9);

        if (player_id == 0)
        {
            // Add player0's operation
            s.add_operation_of_player(0, build_tower0);
            // Apply player0's operation
            s.apply_operations_of_player(0);
            // Add player1's operation
            s.add_operation_of_player(1, build_tower1);
            // Apply player1's operation
            s.apply_operations_of_player(1);
            // Next round
            if (s.next_round() != GameState::Running)
                break;
        }
        else
        {
            // Add player1's operation
            s.add_operation_of_player(1, build_tower1);
            // Apply player1's operation
            s.apply_operations_of_player(1);
            // Next round
            if (s.next_round() != GameState::Running)
                break;
            // Add player0's operation
            s.add_operation_of_player(0, build_tower0);
            // Apply player0's operation
            s.apply_operations_of_player(0);
        }
    }

    std::vector<Operation> ops; // Operations to be returned

    {
        // Make decision with simulated results
    }

    return ops;
}

int main()
{
    // Run the game with the simple AI
    run_with_ai(simple_ai);

    // Run the game with the advanced AI
    // run_with_ai(advanced_ai);

    return 0;
}