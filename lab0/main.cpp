#include <iostream>
#include <vector>
#include <map>
#include "GameInterfaces.h"
#include "StoryEntities.h"

using namespace std;

// 2. SRP: GameEngine manages game execution and story flow
class GameEngine {
private:
    Player* player;
    map<int, StoryNode*> storyGraph;
    IOutputWriter* writer; // DIP

public:
    GameEngine(Player* p, IOutputWriter* w) {
        player = p;
        writer = w;
    }

    ~GameEngine() {
        for (auto pair : storyGraph) {
            delete pair.second;
        }
    }

    void addNode(StoryNode* node) {
        storyGraph[node->getId()] = node;
    }

    void start(int startNodeId) {
        writer->write("FAF-CITY: CHOOSE YOUR DESTINY");
        int currentNodeId = startNodeId;

        while (!player->getIsGameOver()) {
            player->displayStats();

            if (storyGraph.find(currentNodeId) == storyGraph.end()) {
                writer->write("Error: Story node missing!");
                break;
            }

            StoryNode* currentNode = storyGraph[currentNodeId];
            currentNode->displayNode();

            if (currentNode->getChoiceCount() == 0) {
                break;
            }

            cout << "\nChoose option: ";
            int userChoice;
            cin >> userChoice;

            if (userChoice < 1 || userChoice > currentNode->getChoiceCount()) {
                cout << "Invalid option, try again.\n";
                continue;
            }

            Choice selected = currentNode->getChoice(userChoice - 1);

            // Requirements checks
            if (!selected.requiredItem.empty() && !player->hasItem(selected.requiredItem)) {
                cout << "\nYou do not have the required item: " << selected.requiredItem << "\n";
                continue;
            }

            if (selected.requiredIntel > 0 && player->getIntelligence() < selected.requiredIntel) {
                cout << "\nYou are not smart enough for this choice! Need INT: " << selected.requiredIntel << "\n";
                continue;
            }

            // Apply choice effects (OCP)
            for (IActionEffect* effect : selected.effects) {
                effect->apply(*player);
            }

            currentNodeId = selected.nextNodeId;
        }

        // Display final result
        cout << "\n";
        if (!player->getEndingMessage().empty()) {
            writer->write(player->getEndingMessage());
        } else {
            writer->write("THE END - Thanks for playing!");
        }
    }
};

int main() {
    string name;
    cout << "Enter your hero name: ";
    cin >> name;

    Player* player = new Player(name);
    IOutputWriter* writer = new ConsoleWriter();
    GameEngine engine(player, writer);

    // --- SCENE 1: START (3 CHOICES) ---
    StoryNode* node1 = new StoryNode(1, "You wake up in a rainy alley in Sector 7. A merchant sells gear, a hacker waves at you, and a gang member stares you down.");
    node1->addChoice("Buy a Neural Chip ($80)", 2, { new StatEffect(0, -80, 15, 0), new ItemEffect("Neural Chip") });
    node1->addChoice("Buy a Medkit and Stun Gun ($90)", 3, { new StatEffect(0, -90, 0, 5), new ItemEffect("Medkit"), new ItemEffect("Stun Gun") });
    node1->addChoice("Follow the street hacker to a bar", 4, { new StatEffect(0, 0, 5, 10) });

    // --- SCENE 2: HACKER PATH (3 CHOICES) ---
    StoryNode* node2 = new StoryNode(2, "With your Neural Chip, you spot a weak point in the city bank network.");
    node2->addChoice("Hack bank servers directly", 5, {}, "", 20); // Needs INT 20
    node2->addChoice("Sell the security flaw to a Netrunner clan", 6, { new StatEffect(0, 500, 5, 10) });
    node2->addChoice("Apply for a C++ developer job at Megacorp", 7, { new StatEffect(0, 200, 10, 5) });

    // --- SCENE 3: FIGHTER PATH (3 CHOICES) ---
    StoryNode* node3 = new StoryNode(3, "A group of street bandits approaches you to steal your money.");
    node3->addChoice("Use Stun Gun on their leader", 8, { new StatEffect(-10, 300, 0, 25) }, "Stun Gun");
    node3->addChoice("Give them $50 and run away", 9, { new StatEffect(-10, -50, 0, -5) });
    node3->addChoice("Bribe them with your Medkit for passage", 10, { new StatEffect(0, 0, 0, 10) }, "Medkit");

    // --- SCENE 4: BAR PATH (3 CHOICES) ---
    StoryNode* node4 = new StoryNode(4, "At the bar, a fixer offers you a risky job stealing corporate data.");
    node4->addChoice("Accept the corporate heist job", 11, { new StatEffect(0, 0, 0, 15) });
    node4->addChoice("Decline and work as bar security", 12, { new StatEffect(-10, 200, 0, 5) });
    node4->addChoice("Steal the fixer's cyber-deck while he looks away", 13, { new StatEffect(0, 0, 10, -10), new ItemEffect("Cyber-Deck") });

    // --- SECOND LAYER NODES (EXACTLY 3 CHOICES EACH) ---

    // Node 5
    StoryNode* node5 = new StoryNode(5, "Your hack works completely! You control millions in digital currency.");
    node5->addChoice("Transfer funds and buy an orbital palace", 14, {});
    node5->addChoice("Erase all debt records for Sector 7 citizens", 15, { new StatEffect(0, 0, 0, 50) });
    node5->addChoice("Buy out Megacorp stock to become CEO", 16, {});

    // Node 6
    StoryNode* node6 = new StoryNode(6, "The Netrunners pay you and offer you a full membership.");
    node6->addChoice("Lead the Netrunner clan into a city war", 15, {});
    node6->addChoice("Invest the money into a tech company", 16, {});
    node6->addChoice("Gamle all earnings at an underground casino", 17, { new StatEffect(0, -500, 0, 0) });

    // Node 7
    StoryNode* node7 = new StoryNode(7, "You get hired as a coder and work hard every single day.");
    node7->addChoice("Outsmart your boss to take the CEO chair", 16, {});
    node7->addChoice("Work 100-hour weeks without stopping", 18, { new StatEffect(-40, 500, 10, 0) });
    node7->addChoice("Build an indie game on weekends", 19, { new StatEffect(10, 1000, 20, 20) });

    // Node 8
    StoryNode* node8 = new StoryNode(8, "You defeated the bandit leader! The street clans respect your strength.");
    node8->addChoice("Unite the street clans under your rule", 15, {});
    node8->addChoice("Demand monthly protection taxes from gangs", 16, {});
    node8->addChoice("Retire from violence and guard the local neighborhood", 19, {});

    // Node 9
    StoryNode* node9 = new StoryNode(9, "You escaped, but you lost money and street respect.");
    node9->addChoice("Beg for food in Sector 7 alleys", 17, {});
    node9->addChoice("Try to steal money from a market stall", 18, { new StatEffect(-50, 20, 0, 0) });
    node9->addChoice("Look for an entry-level cleaning job at Megacorp", 7, { new StatEffect(0, 50, 0, 0) });

    // Node 10
    StoryNode* node10 = new StoryNode(10, "The bandits accept your Medkit and leave you alone.");
    node10->addChoice("Head to the bar to look for work", 4, {});
    node10->addChoice("Search the street trash for spare parts", 13, { new ItemEffect("Cyber-Deck") });
    node10->addChoice("Sleep on a park bench to recover energy", 19, { new StatEffect(20, 0, 0, 0) });

    // Node 11
    StoryNode* node11 = new StoryNode(11, "You sneak into the corporate vault, but security turrets active!");
    node11->addChoice("Use a Cyber-Deck to disable turret code", 14, {}, "Cyber-Deck");
    node11->addChoice("Use a Medkit to patch wounds and run", 19, { new StatEffect(10, 0, 0, 0) }, "Medkit");
    node11->addChoice("Try to charge through the turret fire", 20, { new StatEffect(-100, 0, 0, 0) });

    // Node 12
    StoryNode* node12 = new StoryNode(12, "You work quiet security shifts at the bar and save your money.");
    node12->addChoice("Buy the bar from the owner", 16, {});
    node12->addChoice("Live a simple, peaceful street life", 19, {});
    node12->addChoice("Use savings to buy a Neural Chip", 2, { new StatEffect(0, -80, 15, 0), new ItemEffect("Neural Chip") });

    // Node 13
    StoryNode* node13 = new StoryNode(13, "You obtained a Cyber-Deck! You are ready for high-tier hacking.");
    node13->addChoice("Hack bank servers directly", 5, {}, "", 15);
    node13->addChoice("Accept the corporate heist job", 11, {});
    node13->addChoice("Sell the Cyber-Deck for quick cash", 6, { new StatEffect(0, 300, 0, 0) });

    // --- TERMINAL ENDING NODES (0 CHOICES - END OF GAME) ---
    StoryNode* node14 = new StoryNode(14, "TECH BILLIONAIRE ENDING: You hacked the system and retired in ultimate luxury!");
    StoryNode* node15 = new StoryNode(15, "CYBERPUNK LEGEND ENDING: You became the undisputed ruler of Sector 7!");
    StoryNode* node16 = new StoryNode(16, "CORPO CEO ENDING: You climbed to the top and now control Megacorp!");
    StoryNode* node17 = new StoryNode(17, "BROKE ENDING: You lost your funds and ended up broke in FAF-City.");
    StoryNode* node18 = new StoryNode(18, "BURNT-OUT ENDING: You made money, but completely destroyed your health.");
    StoryNode* node19 = new StoryNode(19, "PERFECT ENDING: You balanced work, health, and wealth to live happily!");
    StoryNode* node20 = new StoryNode(20, "YOU DIED: You took too much damage from automated turrets.");

    // --- REGISTER ALL 20 NODES ---
    for (int i = 1; i <= 20; i++) {
        // Adding each node safely
    }
    engine.addNode(node1);
    engine.addNode(node2);
    engine.addNode(node3);
    engine.addNode(node4);
    engine.addNode(node5);
    engine.addNode(node6);
    engine.addNode(node7);
    engine.addNode(node8);
    engine.addNode(node9);
    engine.addNode(node10);
    engine.addNode(node11);
    engine.addNode(node12);
    engine.addNode(node13);
    engine.addNode(node14);
    engine.addNode(node15);
    engine.addNode(node16);
    engine.addNode(node17);
    engine.addNode(node18);
    engine.addNode(node19);
    engine.addNode(node20);

    // Start game
    engine.start(1);

    // Clean memory
    delete player;
    delete writer;

    return 0;
}