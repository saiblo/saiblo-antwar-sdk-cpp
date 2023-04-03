/**
 * @file control.hpp
 * @author Jingxuan Liu, Yufei Li
 * @brief An integrated module of IO and encapsulated game state management.
 * @date 2023-04-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <vector>
#include "game_info.hpp"
#include "io.hpp"

/**
 * @brief An integrated module of IO and game state management with simple interfaces
 * for your convenience. Focus on the information of current game state (e.g. get the
 * probability of an ant's movement) and make your decisions, without caring much about
 * tedious details of IO.
 */
class Controller
{
private:
    GameInfo info;                              ///< Current game information
    std::vector<Operation> self_operations;     ///< Self operations which are about to be sent
    std::vector<Operation> opponent_operations; ///< Opponent's operations received from judger

    /**
     * @brief Check if a BuildTower operation at the same position has been added to your operations.
     * @return Whether there is a BuildTower operation at (x, y) in self_operations.
     */
    bool has_build_tower_operation_at(int x, int y) const
    {
        return std::any_of(self_operations.begin(), self_operations.end(), [x, y](const Operation &op)
                           { return op.type == BuildTower && op.arg0 == x && op.arg1 == y; });
    }

    /**
     * @brief Check if a UpgradeTower or DowngradeTower operation of the same tower id has been added to your operations.
     * @return Whether there is a UpgradeTower or DowngradeTower operation of 'tower_id' in self_operations.
     */
    bool has_upgrade_or_downgrade_tower_operation_of(int tower_id) const
    {
        return std::any_of(self_operations.begin(), self_operations.end(), [tower_id](const Operation &op)
                           { return (op.type == UpgradeTower || op.type == DowngradeTower) && op.arg0 == tower_id; });
    }
    
    /**
     * @brief Check if a base-related operation has been added to your operations.
     * @return Whether there is a base-related operation in self_operations.
     */
    bool has_base_related_operation() const
    {
        return std::any_of(self_operations.begin(), self_operations.end(), [](const Operation &op)
                           { return op.type == UpgradeGeneratedAnt || op.type == UpgradeGenerationSpeed; });
    }

    /* Updating process after calling read_round_info() */

    /**
     * @brief Update "info.towers" with vector "new_towers" and reset "info.next_tower_id".
     * @param new_towers The vector of towers (read from Judger) for updating. 
     */
    void update_towers(std::vector<Tower>& new_towers)
    {
        info.towers = std::move(new_towers);
        info.next_tower_id = info.towers.empty() ? 0 : info.towers.back().id + 1;
    }

    /**
     * @brief Update "info.ants" with vector "new_ants" and reset "info.next_ant_id".
     * @param new_ants The vector of ants (read from Judger) for updating. 
     */
    void update_ants(const std::vector<Ant>& new_ants)
    {
        for (auto& ant : new_ants)
            update_ant(ant);
        info.next_ant_id = info.ants.empty() ? 0 : info.ants.back().id + 1;
    }

    /**
     * @brief Update "info.ants" with ant "a". 
     * @param a The given ant for updating. 
     * @details Given ant "a", find in vector "ants" if there is an ant "b" having the same ID as "a".
     *          If found "b" (which means "a" is not newly generated), then update "b" with "a".
     *          If not found "b" (which means "a" is newly generated), then emplace "a" at the back of
     *          vector "info.ants". 
     */
    void update_ant(const Ant& a)
    {
        auto it = std::find_if(info.ants.begin(), info.ants.end(), [a](const Ant &x)
                          { return x.id == a.id; });
        if (it != info.ants.end()) // not newly generated
        {
            if (!(it->x == a.x && it->y == a.y))
                it->path.push_back(get_direction(it->x, it->y, a.x, a.y));
            it->x = a.x, it->y = a.y, it->hp = a.hp, it->age = a.age, it->state = a.state;
        }
        else // newly generated
        {
            info.ants.emplace(it, a);
        }
    }

    /**
     * @brief Update "info.coins".
     * @param coin0 The number of coins of player0 (read from Judger).
     * @param coin1 The number of coins of player1 (read from Judger).
     * @see GameInfo::set_coin for details. 
     */
    void update_coins(int coin0, int coin1)
    {
        info.set_coin(0, coin0);
        info.set_coin(1, coin1);
    }

    /**
     * @brief Update health points (HP) of "info.base[0]" and "info.base[1]".
     * @param hp0 The hp of player0's base (read from Judger).
     * @param hp1 The hp of player1's base (read from Judger).
     * @see GameInfo::set_base_hp for details.  
     */
    void update_bases_hp(int hp0, int hp1)
    {
        info.set_base_hp(0, hp0);
        info.set_base_hp(1, hp1);
    }

    /**
     * @brief Construct a new Controller object with given init info.
     */
    Controller(InitInfo init_info)
        : info(init_info.second), self_player_id(init_info.first) {}

public:
    const int self_player_id; ///< Your player ID

    /**
     * @brief Construct a new Controller object. Read initializing information from
     *        judger and initialize.
     */
    Controller(): Controller(read_init_info()) {}

    /**
     * @brief Get information about current game state.
     * @return A read-only (constant) reference to the current GameInfo object.
     */
    const GameInfo& get_info() const
    {
        return info;
    }

    /**
     * @brief Get added operations.
     * @return A read-only (constant) reference to self operations.
     */
    const std::vector<Operation>& get_self_operations() const
    {
        return self_operations;
    }

    /**
     * @brief Get saved opponent's operations.
     * @return A read-only (constant) reference to opponent operations.
     */                                                                                                                                                                                                      
    const std::vector<Operation>& get_opponent_operations() const
    {
        return opponent_operations;
    }

    /**
     * @brief Read round information from judger and update current game state.
     */
    void read_round_info()
    {
        // 1. Read
        auto result = ::read_round_info();
        // 2. Update
        // 1) Towers
        update_towers(result.towers);
        // 2) Ants and Pheromone
        update_ants(result.ants);
        info.global_pheromone_attenuation();
        info.update_pheromone_for_ants();
        info.clear_dead_and_succeeded_ants();
        // 3) Coins and Bases
        update_coins(result.coin0, result.coin1);
        update_bases_hp(result.hp0, result.hp1);
        // 3. Start Next Round
        // 1) update round number
        info.round = result.round;
        // 2) count down super weapons' cd
        info.count_down_super_weapons_cd();
        // 3) clear operations
        self_operations.clear();
        opponent_operations.clear();
    }

    /**
     * @brief Read opponent's operations from judger and overwrites "opponent_operations".
     */
    void read_opponent_operations()
    {
        opponent_operations = ::read_opponent_operations();
    }

    /**
     * @brief Apply all the operations in "opponent_operations" to current game state. 
     */
    void apply_opponent_operations()
    {
        // 1) count down opponent's super weapons' left-time
        info.count_down_super_weapons_left_time(!self_player_id);
        // 2) apply opponent's operations
        for (auto& op: opponent_operations)
            info.apply_operation(!self_player_id, op);
        // 3) apply active super weapons
        info.apply_active_super_weapons(!self_player_id);
    }

    /**
     * @brief Try adding an operation to "self_operations". The operation will be constructed with the given type 
     *        and arguments. This function will check validness of the operation and add it to "self_operations" if valid.  
     * @param type The type of the operation (see #OperationType). 
     * @param arg0 The first argument of the operation.
     * @param arg1 (optional) The second argument of the operation.
     * @return Whether the operation is added successfully.
     */
    bool append_self_operation(OperationType type, int arg0 = -1, int arg1 = -1)
    {
        return append_self_operation(Operation(type, arg0, arg1));
    }

    /**
     *  @brief Try adding an operation to "self_operations". The operation has been constructed elsewhere.
     *         This function will check validness of the operation and add it to "self_operations" if valid.  
     *  @param op The operation to be added.
     *  @return Whether the operation is added successfully.
     */
    bool append_self_operation(Operation op)
    {
        // Check building tower
        if (op.type == OperationType::BuildTower && has_build_tower_operation_at(op.arg0, op.arg1))
            return false;
        // Check upgrading tower and downgrading tower
        if ((op.type == OperationType::UpgradeTower || op.type == OperationType::DowngradeTower)
            && has_upgrade_or_downgrade_tower_operation_of(op.arg0))
            return false;
        // Check upgrading base
        if ((op.type == OperationType::UpgradeGeneratedAnt || op.type == OperationType::UpgradeGenerationSpeed) 
            && has_base_related_operation())
            return false;
        
        // Check operation validness
        if (!info.is_operation_valid(self_player_id, op))
            return false;
        
        // Check if the current operation list is affordable (including the new one)
        self_operations.emplace_back(std::move(op));
        if (!info.check_affordable(self_player_id, self_operations))
        {
            // Unaffordable. Remove the new operation.
            self_operations.pop_back();
            return false;
        }

        // Add the operation
        return true;
    }

    /**
     * @brief Apply all the operations in "self_operations" to current game state. 
     */
    void apply_self_operations()
    {
        // 1) count down self's long-lasting weapons' left-time
        info.count_down_super_weapons_left_time(self_player_id);
        // 2) apply self operations
        for (auto& op: self_operations)
            info.apply_operation(self_player_id, op);
        // 3) apply active super weapons
        info.apply_active_super_weapons(self_player_id);
    }

    /**
     * @brief Send all the operations in "self_operations" (i.e. print to stdout)
     */ 
    void send_self_operations() const
    {
        send_operations(self_operations);        
    }
};