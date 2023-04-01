#include "../include/control.hpp"
#include "../include/simulate.hpp"

// Construct a Controller object and initialize it
Controller c;

// Game process when you are player 0
void game_process0()
{
    while (true)
    {
        std::cerr << "add operations" << std::endl;
        // Create a simulator
        Simulator s(c.get_info());
        // Simulate 10 rounds
        for (int i = 0; i < 10; ++i)
        {
            Operation build_tower0(OperationType::BuildTower, 5, 9);
            Operation build_tower1(OperationType::BuildTower, 13, 9);

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
        // Add your operations here
        c.append_self_operation(BuildTower, 5, 9);
        c.append_self_operation(BuildTower, 5, 3);
        c.append_self_operation(BuildTower, 5, 15);

        std::cerr << "send operations" << std::endl;
        c.send_self_operations(); // Send your operations to judger

        std::cerr << "apply self operations" << std::endl;
        c.apply_self_operations(); // Apply your operations to game state

        std::cerr << "read opponent operations" << std::endl;
        c.read_opponent_operations(); // Read opponent operations from judger

        std::cerr << "apply opponent operations" << std::endl;
        c.apply_opponent_operations(); // Apply opponent operations to game state

        std::cerr << "read round data" << std::endl;
        c.read_round_info(); // Read round data from judger
    }
}

// Game process when you are player 1
void game_process1()
{
    while (true)
    {
        std::cerr << "read opponent operations" << std::endl;
        c.read_opponent_operations(); // Read opponent operations from judger

        std::cerr << "apply opponent operations" << std::endl;
        c.apply_opponent_operations(); // Apply opponent operations to game state

        std::cerr << "add operations" << std::endl;
        // Create a simulator
        Simulator s(c.get_info());
        // Simulate 10 rounds
        for (int i = 0; i < 10; ++i)
        {
            Operation build_tower0(OperationType::BuildTower, 5, 9);
            Operation build_tower1(OperationType::BuildTower, 13, 9);

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
        // Add your operations here
        c.append_self_operation(BuildTower, 13, 9);
        c.append_self_operation(BuildTower, 13, 3);
        c.append_self_operation(BuildTower, 13, 15);

        std::cerr << "send operations" << std::endl;
        c.send_self_operations(); // Send your operations to judger

        std::cerr << "apply self operations" << std::endl;
        c.apply_self_operations(); // Apply your operations to game state

        std::cerr << "read round data" << std::endl;
        c.read_round_info(); // Read round data from judger
    }
}

int main()
{
    // Check your player ID and switch to different game process
    if (c.self_player_id == 0)
    {
        std::cerr << "Player 0 initialized" << std::endl;
        game_process0();
    }
    else
    {
        std::cerr << "Player 1 initialized" << std::endl;
        game_process1();
    }
}