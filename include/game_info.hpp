/**
 * @file game_info.hpp
 * @author Jingxuan Liu, Yufei li
 * @brief A bottom module used for game state management.
 * @date 2023-04-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <vector>
#include <utility>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include "common.hpp"
#include "optional.hpp"

/**
 * @brief A module used for game state management, providing interfaces for accessing and modifying 
 * various types of information such as Entity, Economy, Pheromone, SuperWeapon and Operation. 
 */
struct GameInfo
{
    int round;                                      ///< Current round number
    std::vector<Tower> towers;                      ///< All towers on the map
    std::vector<Ant> ants;                          ///< All ants on the map
    Base bases[2];                                  ///< Bases of both sides: "bases[player_id]"
    int coins[2];                                   ///< Coins of both sides: "coins[player_id]"
    double pheromone[2][MAP_SIZE][MAP_SIZE];        ///< Pheromone of each point on the map: "pheromone[player_id][x][y]"
    std::vector<SuperWeapon> super_weapons;         ///< Super weapons being used
    int super_weapon_cd[2][SuperWeaponCount];       ///< Super weapon cooldown of both sides: "super_weapon_cd[player_id]"
    
    int next_ant_id;                                ///< ID of the next generated ant.
    int next_tower_id;                              ///< ID of the next built tower.

    GameInfo(unsigned long long seed)
        : round(0), bases{Base(0), Base(1)}, coins{COIN_INIT, COIN_INIT},
          super_weapon_cd{}, next_ant_id(0), next_tower_id(0)
    {
        // Initialize pheromone
        Random random(seed);
        for(int i = 0; i < 2; i++)
            for(int j = 0; j < MAP_SIZE; j++)
                for(int k = 0; k < MAP_SIZE; k++)
                    pheromone[i][j][k] = random.get() * std::pow(2, -46) + 8;
    }

    /* Getters */

    /**
     * @brief Find no more than one element in the given vector for which a predicate is true.
     * @param v A vector.
     * @param pred A predicate. 
     * @return An optional object whose value satisfies "pred" or nullopt if not found. 
     */
    template<typename T, typename Pred>
    optional<T> find_one(const std::vector<T>& v, Pred pred) const
    {
        auto it = std::find_if(v.begin(), v.end(), pred);
        if (it != v.end())
            return make_optional<T>(*it);
        else
            return nullopt;
    }
    
    /**
     * @brief Find all elements in the given vector for which a predicate is true.
     * @param v A vector.
     * @param pred A predicate. 
     * @return A vector of copies of all elements for which "pred" is true. 
     */
    template<typename T, typename Pred>
    std::vector<T> find_all(const std::vector<T>& v, Pred pred) const
    {
        std::vector<T> fit_elems;
        for (const T& e: v)
            if (pred(e))
                fit_elems.emplace_back(e);
        return fit_elems;
    }

    // Ant
    
    /**
     * @brief Get all ants on the map.
     * @return A vector of copies of all ants on the map.
     */
    std::vector<Ant> all_ants() const
    {
        return ants;
    }

    /**
     * @brief Find all ants at a specific point.
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     * @return A vector of copies of all ants at the given point.
     */
    std::vector<Ant> ant_at(int x, int y) const
    {
        return find_all(ants, [x, y](const Ant &a){ return a.x == x && a.y == y; });
    }

    /**
     * @brief Find the ant of a specific ID.
     * @param id The ID of the target ant.
     * @return An optional object whose value is the ant of the given ID or
     *         nullopt if not found.
     */
    optional<Ant> ant_of_id(int id) const
    {
        return find_one(ants, [id](const Ant &a) { return a.id == id; });
    }

    /**
     * @brief Find the ant of a specific ID and get its index in vector "ants".
     * @param id The ID of the target ant.
     * @return The index of the ant in vector "ants" or -1 if not found.
     */
    int ant_of_id_by_index(int id) const
    {
        auto it = std::find_if(ants.begin(), ants.end(), [&](const Ant &a)
                               { return a.id == id; });
        if (it != ants.end())
            return it - ants.begin();
        else
            return -1;
    }

    // Tower
    
    /**
     * @brief Get all towers on the map.
     * @return A vector of copies of all towers on the map.
     */
    std::vector<Tower> all_towers() const
    {
        return towers;
    }

    /**
     * @brief Find the tower at a specific point.
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     * @return An optional object whose value is the tower at the given point or
     *         nullopt if not found.
     */
    optional<Tower> tower_at(int x, int y) const
    {
        return find_one(
            towers, [x, y](const Tower &t) { return t.x == x && t.y == y; });
    }

    /**
     * @brief Find the tower of a specific ID.
     * @param id The ID of the target tower.
     * @return An optional object whose value is the tower of the given ID or
     *         nullopt if not found.
     */
    optional<Tower> tower_of_id(int id) const
    {
        return find_one(
            towers, [id](const Tower &t) { return t.id == id; });
    }

    /* Setters */

    /**
     * @brief Emplace a new tower at the back of vector "towers".
     * @param id The ID of the new tower.
     * @param player The player id of the new tower.
     * @param x The x-coordinate of the new tower.
     * @param y The y-coordinate of the new tower.
     * @param type The type of the new tower (default type = TowerType::Basic).
     */
    void build_tower(int id, int player, int x, int y, TowerType type = TowerType::Basic)
    {
        towers.emplace_back(id, player, x, y, type);
    }

    /**
     * @brief Find the tower of a specific ID, then upgrade it.
     * @param id The ID of the tower about to be upgraded.
     * @param type The target type of upgrading.
     */
    void upgrade_tower(int id, TowerType type)
    {
        auto it = std::find_if(towers.begin(), towers.end(), [&](const Tower &t)
                               { return t.id == id; });
        if (it != towers.end())
        {
            it->upgrade(type);
        }
    }

    /**
     * @brief Find the tower of a specific ID and downgrade it if possible. Otherwise,
     *        erase it from vector "towers".
     * @param id The ID of the tower about to be destroyed.
     */
    void downgrade_or_destroy_tower(int id)
    {
        auto it = std::find_if(towers.begin(), towers.end(), [&](const Tower &t)
                               { return t.id == id; });
        if (it != towers.end())
        {
            if (it->is_downgrade_valid()) // Downgrade
                it->downgrade();
            else // Destroy
                towers.erase(it);
        }
    }

    void upgrade_generation_speed(int player_id)
    {
        bases[player_id].upgrade_generation_speed();
    }

    void upgrade_generated_ant(int player_id)
    {
        bases[player_id].upgrade_generated_ant();
    }

    /**
     * @brief Set the number of coins for a player.
     */
    void set_coin(int player_id, int value)
    {
        coins[player_id] = value;
    }

    /**
     * @brief Update the number of coins for a player.
     */
    void update_coin(int player_id, int change)
    {
        coins[player_id] += change;
    }

    /**
     * @brief Set the hp of base for a player.
     */
    void set_base_hp(int player_id, int value)
    {
        bases[player_id].hp = value;
    }

    /**
     * @brief Update the hp of base for a player.
     */
    void update_base_hp(int player_id, int change)
    {
        bases[player_id].hp += change;
    }

    /* Ants and pheromone updaters. */

    /**
     * @brief Clear ants of state "Success", "Fail" or "TooOld".
     */
    void clear_dead_and_succeeded_ants()
    {
        for (auto it = ants.begin(); it != ants.end();)
        {
            if (it->state == AntState::Success || it->state == AntState::Fail || it->state == AntState::TooOld)
                it = ants.erase(it);
            else
                ++it;
        }
    }

    /**
     * @brief Update pheromone for each ant.
     */
    void update_pheromone_for_ants()
    {
        for (const Ant& ant : ants)
            update_pheromone(ant);
    }

    /**
     * @brief Update pheromone based on the state of an ant.
     * @param ant The given ant for updating.
     */
    void update_pheromone(const Ant &ant)
    {
        // Parameters for the algorithm
        static constexpr double TAU[] = {0.0, 10.0, -5, -3};
        
        // Do nothing if the ant is alive or frozen
        if (ant.state == AntState::Alive || ant.state == AntState::Frozen)
            return;
        
        // Update pheromone from start to end
        int tau = TAU[ant.state];
        int player = ant.player;
        int x = Base::POSITION[player][0], y = Base::POSITION[player][1];
        bool visited[MAP_SIZE][MAP_SIZE] = {};
        
        for (int move: ant.path)
        {
            // If not visited yet
            if (!visited[x][y])
            {
                visited[x][y] = true; // Mark on the map
                pheromone[player][x][y] += tau; // Update pheromone
                if (pheromone[player][x][y] < PHEROMONE_MIN) // No underflow
                    pheromone[player][x][y] = PHEROMONE_MIN;
            }
            // Move to next position
            x += OFFSET[y % 2][move][0];
            y += OFFSET[y % 2][move][1];
        }

        // Should have reached the end now
        assert(x == ant.x && y == ant.y);
        if (!visited[x][y]) // Update at the current position if not visited yet
        {
            pheromone[player][x][y] += tau;
            if (pheromone[player][x][y] < PHEROMONE_MIN)
                pheromone[player][x][y] = PHEROMONE_MIN;
        }
    }

    /**
     * @brief Global pheromone attenuation.
     */
    void global_pheromone_attenuation()
    {
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < MAP_SIZE; ++j)
                for (int k = 0; k < MAP_SIZE; ++k)
                    pheromone[i][j][k] =
                        PHEROMONE_ATTENUATING_RATIO * pheromone[i][j][k]
                        + (1 - PHEROMONE_ATTENUATING_RATIO) * PHEROMONE_INIT;
    }

    /* Operation checkers and appliers */

    /**
     * @brief Count the number of towers of a player.
     * @param player_id The player.
     * @return The number of towers of the player. 
     */
    int tower_num_of_player(int player_id) const
    {
        return std::count_if(
            towers.begin(),
            towers.end(),
            [player_id](const Tower& tower) {
                return tower.player == player_id;
            }
        );
    }

    /**
     * @brief Check operation validness.
     * @param player_id The player.
     * @param op The operation.
     * @return Whether the operation is valid (i.e. can be applied).
     * @note This function does not check whether the player has enough coins, or
     * there are multiple operations of the same type. See another overload for those checks.
     */
    bool is_operation_valid(int player_id, const Operation& op) const
    {
        switch (op.type)
        {
            case BuildTower:
                return is_highland(player_id, op.arg0, op.arg1)
                       && !tower_at(op.arg0, op.arg1)
                       && !is_shielded_by_emp(player_id, op.arg0, op.arg1);
            case UpgradeTower:
            {
                auto t = tower_of_id(op.arg0);
                return t && t.value().player == player_id
                       && t.value().is_upgrade_type_valid(op.arg1) 
                       && !is_shielded_by_emp(t.value());
            }
            case DowngradeTower:
            {
                auto t = tower_of_id(op.arg0);
                return t && t.value().player == player_id
                       && !is_shielded_by_emp(t.value());
            }
            case UseLightningStorm:
            case UseEmpBlaster:
            case UseDeflector:
            case UseEmergencyEvasion:
                return  is_valid_pos(op.arg0, op.arg1)
                        && super_weapon_cd[player_id][op.type % 10] <= 0;
            case UpgradeGenerationSpeed:
                return bases[player_id].gen_speed_level < 2;
            case UpgradeGeneratedAnt:
                return bases[player_id].ant_level < 2;
            default:
                return false;
        }
    }

    /**
     * @brief Check whether the newly added operation is valid, considering not only the operation itself,
     * but also the operations added before and the player's coins.
     * @param player_id The player.
     * @param ops Operations already added, with the newly added one at the end.
     * @return Whether the operation is valid.
     */
    bool is_operation_valid(int player_id, const std::vector<Operation>& ops, const Operation& new_op) const
    {
        // Check if there are multiple operations of the same type
        bool collide = false;
        switch (new_op.type)
        {
            // At specified position only one tower can be built
            case OperationType::BuildTower:
                collide = std::any_of(ops.begin(), ops.end(), [&](const Operation& op) {
                    return op.type == BuildTower && op.arg0 == new_op.arg0 && op.arg1 == new_op.arg1;
                });
                break;
            // A tower can only be upgraded/downgraded once
            case OperationType::UpgradeTower:
            case OperationType::DowngradeTower:
                collide = std::any_of(ops.begin(), ops.end(), [&](const Operation& op) {
                    return (op.type == UpgradeTower || op.type == DowngradeTower) && op.arg0 == new_op.arg0;
                });
                break;
            // Base can only be upgraded once
            case OperationType::UpgradeGeneratedAnt:
            case OperationType::UpgradeGenerationSpeed:
                collide = std::any_of(ops.begin(), ops.end(), [&](const Operation& op) {
                    return op.type == UpgradeGeneratedAnt || op.type == UpgradeGenerationSpeed;
                });
                break;
            // Super weapon of specified type can only be used once
            case OperationType::UseLightningStorm:
            case OperationType::UseEmpBlaster:
            case OperationType::UseDeflector:
            case OperationType::UseEmergencyEvasion:
                collide = std::any_of(ops.begin(), ops.end(), [&](const Operation& op) {
                    return op.type == new_op.type;
                });
                break;
            // Illegal operation type
            default:
                return false;
        }
        if (collide)
            return false;

        // Check operation validness
        if (!is_operation_valid(player_id, new_op))
            return false;
        
        // Check if the player has enough coins
        std::vector<Operation> new_ops(ops);
        new_ops.push_back(new_op);
        if (!check_affordable(player_id, new_ops))
            return false;

        // Pass all checks. The operation has been added successfully.
        return true;
    }

    /**
     * @brief Get the income of an operation BEFORE applied. The income could be
     * negative, which means the operation costs money.
     * @param player_id The player.
     * @param op The operation.
     * @return How much the player will get when applying the operation.
     */
    int get_operation_income(int player_id, const Operation& op) const
    {
        switch (op.type)
        {
            case BuildTower:
                return -build_tower_cost(tower_num_of_player(player_id));
            case UpgradeTower:
                return -upgrade_tower_cost(op.arg1);
            case DowngradeTower:
            {
                auto t = tower_of_id(op.arg0);
                if (t.value().type == TowerType::Basic) // To be destroyed
                    return destroy_tower_income(tower_num_of_player(player_id));
                else // To be downgraded
                    return downgrade_tower_income(t.value().type);
            }
            case UseLightningStorm:
            case UseEmpBlaster:
            case UseDeflector:
            case UseEmergencyEvasion:
                return -use_super_weapon_cost(op.type % 10);
            case UpgradeGenerationSpeed:
                return -upgrade_base_cost(bases[player_id].gen_speed_level);
            case UpgradeGeneratedAnt:
                return -upgrade_base_cost(bases[player_id].ant_level);
            default:
                return 0;
        }
    }
    
    /**
     * @brief Check whether a player can afford a set of operations.
     * @param player_id The player.
     * @param ops The operation set.
     * @return Whether the player can afford these operations.
     */
    bool check_affordable(int player_id, const std::vector<Operation>& ops) const
    {
        int income = 0, tower_num = tower_num_of_player(player_id);
        for (const Operation& op : ops)
        {
            // Special handling for BuildTower and DowngradeTower, for the cost of
            // BuildTower and DowngradeTower depends on the number of towers of the player.
            switch (op.type)
            {
            case OperationType::BuildTower:
                income -= build_tower_cost(tower_num++);
                break;
            case OperationType::DowngradeTower:
            {
                auto t = tower_of_id(op.arg0);
                if (t.value().type == TowerType::Basic) // To be destroyed
                    income += destroy_tower_income(tower_num--);
                else // To be downgraded
                    income += downgrade_tower_income(t.value().type);
                break;
            }
            default:
                income += get_operation_income(player_id, op);
                break;
            }
        }
        return income + coins[player_id] >= 0;
    }

    /**
     * @brief Change buildings, targets and coin values based on the given operation.
     * @param player_id Whose operation. 
     * @param op The operation to be applied.
     */
    void apply_operation(int player_id, const Operation& op)
    {
        update_coin(player_id, get_operation_income(player_id, op));
        switch (op.type)
        {
            case BuildTower:
                build_tower(next_tower_id++, player_id, op.arg0, op.arg1);
                break;
            case UpgradeTower:
                upgrade_tower(op.arg0, static_cast<TowerType>(op.arg1));
                break;
            case DowngradeTower:
                downgrade_or_destroy_tower(op.arg0);
                break;
            case UseLightningStorm:
            case UseEmpBlaster:
            case UseDeflector:
            case UseEmergencyEvasion:
                use_super_weapon(static_cast<SuperWeaponType>(op.type % 10), player_id, op.arg0, op.arg1);
                break;
            case UpgradeGenerationSpeed:
                upgrade_generation_speed(player_id);
                break;
            case UpgradeGeneratedAnt:
                upgrade_generated_ant(player_id);
                break;
        }
    }

    /* ACO predictors */

    /**
     * @brief Get next moving direction for an ant based on the probability, randomly.
     * @param a The ant.
     * @return Next moving direction. 
     */
    int next_move(const Ant& ant) const
    {
        // Constants
        static constexpr double ETA[] = {1.25, 1.00, 0.75};
        static constexpr int ETA_OFFSET = 1;

        // Data
        int target_x = Base::POSITION[!ant.player][0],
            target_y = Base::POSITION[!ant.player][1];
        int cur_dist = distance(ant.x, ant.y, target_x, target_y);

        // Store weighted and original pheromone
        double phero[6][2] = {};
        static constexpr int WEIGHTED = 0, ORIGINAL = 1;
        std::fill(&phero[0][0], &phero[0][0] + sizeof(phero) / sizeof(double), -1.0); // Init

        // Compute weighted and original pheromone
        for (int i = 0; i < 6; ++i)
        {
            // Neighbor coordinates
            int x = ant.x + OFFSET[ant.y % 2][i][0],
                y = ant.y + OFFSET[ant.y % 2][i][1];
            // Valid: not blocked and not going back
            if ((!ant.path.empty() && ant.path.back() == (i + 3) % 6) || !is_path(x, y))
                continue;
            // Weight (Atrract)
            int next_dist = distance(x, y, target_x, target_y);
            double weight = ETA[next_dist - cur_dist + ETA_OFFSET];
            // Update
            phero[i][WEIGHTED] = weight * pheromone[ant.player][x][y];
            phero[i][ORIGINAL] = pheromone[ant.player][x][y];
        }

        // Get max
        auto p = std::max_element(phero, std::end(phero),
            [phero] (const double ph1[], const double ph2[]) {
                // Check weighted pheromone
                if (ph1[WEIGHTED] != ph2[WEIGHTED])
                    return ph1[WEIGHTED] < ph2[WEIGHTED];
                // Then check original pheromone
                if (ph1[ORIGINAL] != ph2[ORIGINAL])
                    return ph1[ORIGINAL] < ph2[ORIGINAL];
                // If all equals, take one with smaller index as the bigger
                return std::distance(&phero[0][0], ph1) > std::distance(&phero[0][0], ph2);
            });
        
        // Return direction
        return std::distance(phero, p);
    }
    
    /* Caculators for economy */

    /**
     * @brief Calculate the income of detroying a tower for a player.
     * @param tower_num The number of barracks (before destroying) of the player.
     * @return Income of destroying. 
     */
    static int destroy_tower_income(int tower_num)
    {
        return build_tower_cost(tower_num - 1) * TOWER_DOWNGRADE_REFUND_RATIO;
    }

    /**
     * @brief Calculate the income of downgrading a tower for a player.
     * @param type The type of the tower before downgraded.
     * @return Income of downgrading. 
     */
    static int downgrade_tower_income(int type)
    {
        return upgrade_tower_cost(type) * TOWER_DOWNGRADE_REFUND_RATIO;
    }

    /**
     * @brief Calculate the cost of building a tower for a player.
     * @param tower_num The number of towers (before building) of the player.
     * @return Cost of building. 
     */
    static int build_tower_cost(int tower_num)
    {
        return TOWER_BUILD_PRICE_BASE * std::pow(TOWER_BUILD_PRICE_RATIO, tower_num);
    }

    /**
     * @brief Calculate the cost of upgrading a tower.
     * @param type The target type of upgrading.
     * @return Cost of upgrading.
     */
    static int upgrade_tower_cost(int type)
    {
        switch (type)
        {
            // Level 2
            case TowerType::Heavy:
            case TowerType::Quick:
            case TowerType::Mortar:
                return LEVEL2_TOWER_UPGRADE_PRICE;
            // Level 3
            case TowerType::HeavyPlus:
            case TowerType::Ice:
            case TowerType::Cannon:
            case TowerType::QuickPlus:
            case TowerType::Double:
            case TowerType::Sniper:
            case TowerType::MortarPlus:
            case TowerType::Pulse:
            case TowerType::Missile:
                return LEVEL3_TOWER_UPGRADE_PRICE;
        }
        return -1;
    }

    /**
     * @brief Calculate the cost of upgrading a base's generation speed or level of generated ants.
     * @param level Level of the base before upgrading.
     * @return Cost of upgrading.
     */
    static int upgrade_base_cost(int level)
    {
        switch (level)
        {
            case 0: return LEVEL2_BASE_UPGRADE_PRICE;
            case 1: return LEVEL3_BASE_UPGRADE_PRICE;
        }
        return -1;
    }

    /**
     * @brief Calculate the cost of using a super weapon.
     * @param type The type of super weapon.
     * @return Cost of using.
     */
    static int use_super_weapon_cost(int type)
    {
        return SUPER_WEAPON_INFO[type][3];
    }

    /* Super weapons */

    /**
     * @brief Handle the operation of using a super weapon.
     */
    void use_super_weapon(SuperWeaponType type, int player, int x, int y)
    {
        // Add a new super weapon
        super_weapons.emplace_back(type, player, x, y);
        // Reset cd
        super_weapon_cd[player][type] = SUPER_WEAPON_INFO[type][2];
    }

    /**
     * @brief Check whether a point is shielded by EmpBluster for a player.
     * @return Whether the point is shielded.
     */
    bool is_shielded_by_emp(int player_id, int x, int y) const
    {
        return std::any_of(super_weapons.begin(), super_weapons.end(), [player_id, x, y](const SuperWeapon& weapon){
            return weapon.type == EmpBlaster && weapon.player != player_id && weapon.is_in_range(x, y);
        });
    }

    /**
     * @brief Check whether a tower is shielded by EmpBluster.
     * @return Whether the tower is shielded.
     */
    bool is_shielded_by_emp(const Tower& tower) const
    {
        return is_shielded_by_emp(tower.player, tower.x, tower.y);
    }

    /**
     * @brief Check whether an ant is shielded by Deflector for a player.
     * @return Whether the ant is shielded.
     */
    bool is_shielded_by_deflector(const Ant& a) const
    {
        return std::any_of(super_weapons.begin(), super_weapons.end(), [a](const SuperWeapon& weapon){
            return weapon.type == Deflector && weapon.player == a.player && weapon.is_in_range(a.x, a.y);
        });
    }

    /**
     * @brief Count down left_time of super weapons for a player. Clear it if timeout.
     */
    void count_down_super_weapons_left_time(int player_id)
    {
        for (auto it = super_weapons.begin(); it != super_weapons.end(); )
        {
            if (it->player != player_id)
            {
                ++it;
                continue;
            }
            it->left_time--;
            if (it->left_time <= 0)
                it = super_weapons.erase(it);
            else
                ++it;
        }
    }

    /**
     * @brief Apply all active super weapons of a player. 
     */
    void apply_active_super_weapons(int player_id)
    {
        for (const SuperWeapon& super_weapon : super_weapons)
        {
            if (super_weapon.player != player_id)
                continue;
            // Only apply active super weapons (LightningStorm and EmergencyEvasion) 
            if (super_weapon.type == SuperWeaponType::LightningStorm)
            {
                for (Ant &ant : ants)
                {
                    if (super_weapon.is_in_range(ant.x, ant.y) 
                        && ant.player != super_weapon.player)
                    {
                        ant.hp = 0;
                        ant.state = AntState::Fail;
                        update_coin(super_weapon.player, ant.reward());
                    }
                }
            }
            else if (super_weapon.type == SuperWeaponType::EmergencyEvasion)
            {
                for (Ant &ant : ants)
                {
                    if (super_weapon.is_in_range(ant.x, ant.y) 
                        && ant.player == super_weapon.player)
                    {
                        ant.evasion = 2;
                    }
                }
            }
        }
    }

    /**
     * @brief Count down cd of all types of super weapons. 
     */
    void count_down_super_weapons_cd()
    {
        for (int i = 0; i < 2; ++i)
            for (int j = 1; j < 5; ++j)
                super_weapon_cd[i][j] = std::max(super_weapon_cd[i][j] - 1, 0);
    }

    /* For debug */

    /**
     * @brief Print current information to file "info.out".
     */
    void show() const
    {
        std::ofstream fout("info.out");
        fout << "Rounds:" << round << std::endl;
        // Towers
        fout << "Towers:" << std::endl;
        fout << "id\tplayer\tx\ty\ttype\tcd" << std::endl;
        for (auto& tower : towers)
            fout << tower.id << '\t' << tower.player << "\t\t" << tower.x << '\t' << tower.y << '\t' << tower.type <<'\t' << tower.cd << std::endl;
        // Ants
        fout << "Ants:" << std::endl;
        fout << "id\tplayer\tx\ty\thp\tage\tstate" << std::endl;
        for (auto& ant : ants)
            fout << ant.id << '\t' << ant.player << "\t\t" << ant.x << '\t' << ant.y << '\t' << ant.hp << '\t' << ant.age << '\t' << ant.state << std::endl;
        // Coins
        fout << "coin0:" << coins[0] << std::endl;
        fout << "coin1:" << coins[1] << std::endl;
        // Bases
        fout << "base0:" << bases[0].hp << std::endl;
        fout << "base1:" << bases[1].hp << std::endl;

        fout.close();
    }

    /**
     * @brief Dump current information with ofstream
     */
    void dump(std::ofstream& fout) const
    {
        // Round
        fout << round << std::endl;
        // Towers
        fout << towers.size() << std::endl;
        for (auto& tower : towers)
            fout << tower.id << ' '
                 << tower.player << ' '
                 << tower.x << ' '
                 << tower.y << ' '
                 << tower.type << ' '
                 << tower.cd << std::endl;
        // Ants
        fout << ants.size() << std::endl;
        for (auto& ant : ants)
            fout << ant.id << ' '
                 << ant.player << ' '
                 << ant.x << ' '
                 << ant.y  << ' '
                 << ant.hp << ' '
                 << ant.level << ' '
                 << ant.age << ' '
                 << ant.state << std::endl;
        // Coins
        fout << coins[0] << ' ' << coins[1] << std::endl;
        // Hp
        fout << bases[0].hp << ' ' << bases[1].hp << std::endl;
        // Pheromone
        for (int player = 0; player < 2; ++player)
        {
            for (int i = 0; i < MAP_SIZE; ++i)
            {
                for (int j = 0; j < MAP_SIZE; ++j)
                {
                    fout << std::fixed << std::setprecision(4) << pheromone[player][i][j] << ' ';
                }
                fout << std::endl;
            }
        }
    }

    /**
     * @brief Dump current information to file.
     */
    void dump(const char filename[]) const
    {
        std::ofstream fout(filename, std::ios::app);
        dump(fout);
        fout.close();
    }
};
