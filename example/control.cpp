#include "../control.hpp"

// Construct a Controller object and initialize it
Controller c;

// Game process when you are player 0
void game_process0()
{
    while (true)
    {
        std::cerr << "add operations" << std::endl;
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