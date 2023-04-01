# ANTWar C++SDK User Guide

这里是第二十七届智能体大赛 ANTWar 的 C++SDK 使用文档。文档主要包含用户指南和样例 AI ，代码的详细信息请参见 API Reference 文档。希望各位能有良好的参赛体验～

## 1. 预备！
请先前往 GitHub下载 C++SDK 包，确认内含 `control.hpp`、`io.hpp`、`game_info.hpp`、`common.hpp`、`simulate.hpp`、`template.hpp`、`optional.hpp`、`optional-impl.hpp`、`example/`、`Makefile`。

感觉文件有点多？不着急，我们慢慢来~

* `common.hpp` 定义了游戏的各个实体类、操作类、辅助类和其他常数。

* `game_info.hpp` 提供了游戏状态的维护，包括防御塔、工蚁、基地、信息素、超级武器、金币等信息。

* `simulate.hpp` 提供逻辑运行代码，能模拟游戏逻辑的主要流程，包括防御塔攻击、工蚁移动、基地生成工蚁、超级武器的使用、胜负判断等。

* `optional.hpp` 和 `optional-impl.hpp` 用于将 `std::optional<T>` 对 C++ 11 的适配。如你使用的是C++ 17或更高版本，程序将直接使用 C++ 标准库中的  `std::optional<T>` ；如你使用的是 C++ 17 以前版本，将使用 `optional-impl.hpp` 中定义的 `nonstd::optional<T>` 。

* `io.hpp` 提供与Judger的通讯功能，包括读取初始化信息、读取回合信息、读取对手的操作、发送你的操作。

* `control.hpp` 将通讯 IO 模块与游戏状态管理模块进行了集成，你的 AI 程序将利用此模块完成整体的游戏交互流程。

* `template.hpp` 提供了一个简易的 AI 程序的模板，将游戏流程和操作决策分离，从而帮助你更专注于游戏策略的设计。

* `example/` 目录下提供了三个样例 AI 程序，它们分别是：仅使用 `Controller` 的简易 AI 、使用 `Simulator` 辅助决策的进阶 AI 和使用 `template.hpp` 中模板的 AI，我们之后会进行详细介绍。

  

## 2. 开始！

在本节中，我们将介绍类 `Controller` 的使用方法，以及如何用它来实现一个完整游戏流程的简易 AI 。那么我们开始吧！（参考：example/control.cpp）

1. 众所周知，要使用 `Controller` ，得先包含 `control.hpp` 头文件。

    ```cpp
    #include "control.hpp"
    ```

    

2. 然后创建一个全局的 `Controller` 对象，它是你的 AI 实现游戏交互流程的控制器。 `Controller` 的构造函数中会直接调用游戏初始化信息的读取方法，包括你的选手id（先后手信息）和生成初始信息素的随机数种子。

    ```cpp
    Controller c;
    ```

    

3. 再写一个main函数，根据游戏规则，先手和后手有不同的游戏流程，于是我们分成两个函数分别处理。

    ```cpp
    int main()
    {
        if (c.self_player_id == 0)
            game_process0();    // 先手的游戏流程
        else
            game_process1();    // 后手的游戏流程
    }
    ```

    

4. 简易的先手的游戏流程如下：

    ```cpp
    // Game process when you are player 0
    void game_process0()
    {
        while (true) // For each round
        {
            // Add your operations here
            c.append_self_operation(BuildTower, 5, 9);
            c.append_self_operation(BuildTower, 5, 3);
            c.append_self_operation(BuildTower, 5, 15);
    
            c.send_self_operations(); // Send your operations to judger
            c.apply_self_operations(); // Apply your operations to game state
            c.read_opponent_operations(); // Read opponent operations from judger
            c.apply_opponent_operations(); // Apply opponent operations to game state
            c.read_round_info(); // Read round data from judger
        }
    }
    ```
    每回合中，先手__应当按照如下顺序__进行相关函数的调用（若不遵循该顺序可能导致游戏状态的错误维护）：
    
    1. 确定己方的操作。调用 `append_self_operation()` 以将想执行的操作添加到己方操作列表 `self_operations`。
    
    2. 将己方的操作发送给 Judger。调用 `send_self_operations()` 将己方操作列表中的所有操作发送给 Judger。
    
    3. 将己方的操作应用到局面。调用 `apply_self_operations()` 将己方操作列表中的所有操作应用到当前局面，包括：防御塔的建造、升级、拆除，基地的升级，超级武器的使用（各类超级武器会在该函数调用时立即生效）。
    
    4. 读取对方的操作。调用 `read_opponent_operations()` 以读取对方的操作并添加到对方操作列表 `opponent_operations`。
    
    5. 将对方的操作应用到局面。调用 `apply_opponent_operations()` 将操作列表中的所有操作应用到当前局面，包括：防御塔的建造、升级、拆除，基地的升级，超级武器的使用（各类超级武器会在该函数调用时立即生效）。
    
    6. 读取局面信息。此时游戏逻辑已经完成了本回合的结算，并返回结算后的局面信息。调用 `read_round_info` 以读取局面信息，包括：回合数、防御塔、工蚁、金币、基地血量。
    
       
    
    你可能已经注意到了，我们每回合都添加了3个建塔操作，如果这些操作在一回合内全部执行显然是非法的，那么为什么可以这样写呢？其实这里利用了 SDK 在 `append_self_operation()` 实现中的相关特性。在每次调用 `append_self_operation()` 时，`Controller` 都会检查该操作的合法性来决定是否添加至己方操作列表，并返回一个 `bool` 值来表示是否成功添加。
    
    这里进行的检查有：（1）操作本身的合法性：操作类型是否合法、位置是否合法、操作对象是否符合要求等；（2）操作列表的合法性：是否对多个防御塔操作、是否对大本营多次操作、是否有足够的金币执行列表中的所有操作等。
    
    你可以利用这种机制进行某种意义上的“计划”操作，即可以一直给SDK一系列操作，然后让SDK自己决定应用和发送的时机。
    
    ⚠️ __*但是，这种设计有潜在的隐藏选手自身程序问题的风险，因此我们非常建议不要在复杂的逻辑之中应用这种操作。*__
    
    
    
5. 类似地，我们给出后手的游戏流程，相信你已经能看懂了~我们就不再过多解释了。

    ```cpp
    // Game process when you are player 1
    void game_process1()
    {
        while (true)
        {
            c.read_opponent_operations(); // Read opponent operations from judger
            c.apply_opponent_operations(); // Apply opponent operations to game state
    
            // Add your operations here
            c.append_self_operation(BuildTower, 13, 9);
            c.append_self_operation(BuildTower, 13, 3);
            c.append_self_operation(BuildTower, 13, 15);
    
            c.send_self_operations(); // Send your operations to judger
            c.apply_self_operations(); // Apply your operations to game state
            c.read_round_info(); // Read round data from judger
        }
    }
    ```



## 3. 来点高级的呗！

如果你觉得上面的例子还是过分得简单了，你可以考虑在你的 AI 中使用 `Simulator` 模块帮助决策。在本节中，我们会介绍如何在上一节的基础上加入 `Simulator`帮助决策。（参考：example/simulate.cpp）

1. 众所周知，要使用 `Simulator` ，得先包含 `simulate.hpp` 头文件。

   ```cpp
   #include "simulate.hpp"
   ```

   

2. 如果你看到了 `Simulator` 的构造函数，你会发现它需要接受一个 `Controller` 实例，那应该在什么地方构造`Simulate` 实例呢？基本的规则是：**在你需要添加操作之前构造**。这回我们以后手为例进行讲解：

   ```cpp
   // Game process when you are player 1
   void game_process1()
   {
       while (true) // For each round
       {
           c.read_opponent_operations(); // Read opponent operations from judger
           c.apply_opponent_operations(); // Apply opponent operations to game state
   
           // Create a simulator
           Simulator s(c);
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
           {
               const GameInfo& simulation_result = s.get_info();
               // ...  Make decision with simulated results
           }
   
           c.send_self_operations(); // Send your operations to judger
           c.apply_self_operations(); // Apply your operations to game state
           c.read_round_info(); // Read round data from judger
       }
   }
   ```
   
   按照我们刚刚说明的规则，这里构造 `Simulator` 实例并进行相应模拟的位置就很自然了。我们这样做的理由在于 `Simulator` 这个模块的设计动机，即通过模拟游戏后续的若干回合来引导当前回合的操作。在这里，我们从当前回合开始，模拟后续10回合，并根据模拟结果决定当前回合的操作。
   
   
   
3. 我们接下来详细介绍一下 后手的 `Simulator` 的使用流程，如果你从正确的位置开始模拟，则对于接下来的每个回合：

   1. 添加后手的操作。调用 `add_operation_of_player(1， ... )` 将操作添加至后手操作列表（可以为空）。 
   2. 应用后手的操作。调用 `apply_operations_of_player(1， ... )` 将后手操作列表中的所有操作应用到当前局面。
   3. 回合结算。调用 `next_round` 进行回合结算，该函数会返回游戏运行信息（正在运行或某方获胜）。
   4. 添加先手的操作。此时已进入下一回合， 调用 `add_operation_of_player(0， ... )` 将操作添加至先手操作列表（可以为空）。
   5. 应用先手的操作。调用 `apply_operations_of_player(0， ... )` 将先手操作列表中的所有操作应用到当前局面。

   你会发现，这样的顺序可以视作你使用 `Simulator` 代替了游戏逻辑以正确的顺序进行了若干回合的游戏流程。

   

4. 类似地，我们给出先手的 `Simulator` 的使用流程，你需要格外注意它和刚才的代码顺序上的不同之处。

   ```cpp
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
   ```
   
   

## 4. 专注于决策！

阅读完之前3节后，你可能会觉得 AI 程序需要考虑游戏流程这件事过于麻烦。为了解决你的烦恼，我们利用`template.hpp` 中的 `run_with_ai`() 函数给你提供一种专注于决策的优雅实现！（参考 `example/template.cpp`）

1. （梅开三度）众所周知，要使用`template.hpp`中的函数，得先包含 `template.hpp` 头文件。

   ```cpp
   #include "template.hpp"
   ```

   

2. 接下来，我们来看看 `run_with_ai` 函数接受的参数。

   ```cpp
   using AI = std::function<std::vector<Operation>(int, const GameInfo &)>;
   static void run_with_ai(AI ai);
   ```

   这里的 `std::function<std::vector<Operation>(int, const GameInfo &)>` 是一个返回类型为`std::vector<Operation>`，参数类型为`int, const GameInfo &`的函数。因此，你需要写一个这样的函数，它的作用是：给出先后手信息 `int player_id` 和局面信息 `const GameInfo & game_info` ，要求得到一个操作序列 `std::vector<Operation>`，这就是我们“专注于决策”的意义。换言之，**你的 AI 程序只需要完成一个函数**。

   将写好的 AI 的函数作为参数传给 `run_with_ai()` 即可实现决策和游戏流程的组合，进而完成整个 AI 程序的开发。

   

3. 以下给出一个 AI 函数的示例。

   ```cpp
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
   ```

   这个最简单的 AI 函数始终尝试在固定位置建塔，对于双方分别尝试不同的建塔位置（它其实也利用了之前所述的“计划”机制来保证操作的合法性）。你可以在此基础上改进 AI 函数（比如使用 `Simulator` ，参考`example/template.cpp` 中的 `advanced_ai()`函数）以实现更加高级的策略。 



## 5. 我什么都能写！

什么？你说你什么都能写，就是不想写 IO ？那可能只有 `io.hpp` 符合你的口味了。 `io.hpp` 中实现了和 Judger 通信的各个 IO 功能函数，具体如下：

* `read_init_info()`：读取对局初始信息。
* `read_round_info()`：读取回合结算信息。
* `read_opponent_operations()`：读取对手的操作。
* `send_operations()`：发送己方的操作。

什么？你连这些都不想用？那你再看看下面这几个函数吧，我们也就帮你帮到这儿了~

* `object_length()`：获取序列化后的字节数。
* `convert_to_big_endian()`：将整数转换为大端序。
* `print_header()`：输出" 4 + N "协议中的前4个字节。



## 6. 其它

1. 关于 `Makefile`：你可以将你的源文件所在目录加入到 `SOURCEDIRS` 中，然后在 `Makefile` 所在目录下执行 `make {your_source_file_name}` 即可编译你的源文件。文件名可以是相对于 `Makefile` 的相对路径或绝对路径。注意文件名不要带后缀名。编译结果将输出在源文件的同级目录下。例如：

   ```bash
   make example/template
   ```

   将编译 `example/template.cpp` 并输出 `template` 可执行文件。