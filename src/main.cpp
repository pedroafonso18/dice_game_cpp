#include <raylib.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <map>

struct Player {
    std::string name;
    int tries;
    int bestScore;
};

struct ScoreEntry {
    std::string name;
    int bestScore;
    int gamesPlayed;
};

struct Animation {
    Texture2D texture;
    int frames;
    float frameTime;
    float currentTime;
    int currentFrame;
    bool isPlaying;
    int frameWidth;
    int frameHeight;
    float scale;
};

std::map<std::string, ScoreEntry> loadScoreboard() {
    std::map<std::string, ScoreEntry> scoreboard;
    std::ifstream file("scoreboard.txt");
    std::string line;
    
    while (std::getline(file, line)) {
        size_t pos = line.find(',');
        if (pos != std::string::npos) {
            std::string name = line.substr(0, pos);
            size_t pos2 = line.find(',', pos + 1);
            if (pos2 != std::string::npos) {
                int bestScore = std::stoi(line.substr(pos + 1, pos2 - pos - 1));
                int gamesPlayed = std::stoi(line.substr(pos2 + 1));
                scoreboard[name] = {name, bestScore, gamesPlayed};
            }
        }
    }
    return scoreboard;
}

void saveScoreboard(const std::map<std::string, ScoreEntry>& scoreboard) {
    std::ofstream file("scoreboard.txt");
    for (const auto& entry : scoreboard) {
        file << entry.second.name << "," << entry.second.bestScore << "," << entry.second.gamesPlayed << "\n";
    }
}

void resetGame(std::vector<Player>& players, int& currentPlayer, int& targetNumber, 
              std::string& message, std::string& hint, int maxNumber, std::string& currentState,
              Animation& diceAnimation) {
    for (auto& player : players) {
        player.tries = 0;
    }
    currentPlayer = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, maxNumber);
    targetNumber = dis(gen);
    message = "Vez de " + players[currentPlayer].name + ". Adivinhe o número (1-" + std::to_string(maxNumber) + "):";
    hint = "";
    currentState = "game";
    diceAnimation.isPlaying = true;
    diceAnimation.currentTime = 0.0f;
    diceAnimation.currentFrame = 0;
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Jogo de Dados");
    SetTargetFPS(60);

    std::vector<Player> players;
    int currentPlayer = 0;
    int targetNumber = 0;
    bool gameStarted = false;
    bool waitingForGuess = false;
    std::string inputText = "";
    std::string message = "Digite o número de jogadores:";
    std::string currentState = "setup";
    std::string hint = "";
    int maxNumber = 6;
    int numDice = 1;
    bool showScoreboard = false;
    
    float titleScale = 1.0f;
    float titleScaleDirection = 0.002f;
    Color titleColor = {41, 128, 185, 255};
    float messageOpacity = 1.0f;
    float messageOpacityDirection = -0.01f;

    Font gameFont = LoadFontEx("resources/font.ttf", 32, 0, 0);
    if (gameFont.texture.id == 0) {
        gameFont = GetFontDefault();
    }

    Animation diceAnimation = {
        LoadTexture("resources/dice.gif"),
        6,
        0.05f,
        0.0f,
        0,
        false,
        0,
        0,
        0.5f
    };

    if (diceAnimation.texture.id != 0) {
        diceAnimation.frameWidth = diceAnimation.texture.width;
        diceAnimation.frameHeight = diceAnimation.texture.height;
    }

    auto scoreboard = loadScoreboard();

    while(!WindowShouldClose()) {
        titleScale += titleScaleDirection;
        if (titleScale > 1.05f || titleScale < 0.95f) {
            titleScaleDirection *= -1;
        }

        messageOpacity += messageOpacityDirection;
        if (messageOpacity < 0.7f || messageOpacity > 1.0f) {
            messageOpacityDirection *= -1;
        }

        if (currentState == "game" && !diceAnimation.isPlaying) {
            diceAnimation.isPlaying = true;
            diceAnimation.currentTime = 0.0f;
            diceAnimation.currentFrame = 0;
        }

        if (diceAnimation.isPlaying) {
            diceAnimation.currentTime += GetFrameTime();
            if (diceAnimation.currentTime >= diceAnimation.frameTime) {
                diceAnimation.currentTime = 0.0f;
                diceAnimation.currentFrame = (diceAnimation.currentFrame + 1) % diceAnimation.frames;
            }
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (currentState == "setup") {
                int numPlayers = std::stoi(inputText);
                for (int i = 0; i < numPlayers; i++) {
                    players.push_back({"Jogador " + std::to_string(i + 1), 0, 0});
                }
                currentState = "difficulty";
                message = "Escolha a dificuldade (1 ou 2 dados):";
                inputText = "";
            }
            else if (currentState == "difficulty") {
                numDice = std::stoi(inputText);
                if (numDice == 1 || numDice == 2) {
                    maxNumber = numDice * 6;
                    currentState = "name_input";
                    message = "Digite o nome para " + players[currentPlayer].name + ":";
                    inputText = "";
                } else {
                    message = "Por favor, escolha 1 ou 2 dados:";
                    inputText = "";
                }
            }
            else if (currentState == "name_input") {
                if (!inputText.empty()) {
                    players[currentPlayer].name = inputText;
                    if (scoreboard.find(inputText) != scoreboard.end()) {
                        players[currentPlayer].bestScore = scoreboard[inputText].bestScore;
                    }
                    currentPlayer++;
                    if (currentPlayer >= players.size()) {
                        currentState = "game";
                        currentPlayer = 0;
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<> dis(1, maxNumber);
                        targetNumber = dis(gen);
                        message = "Vez de " + players[currentPlayer].name + ". Adivinhe o número (1-" + std::to_string(maxNumber) + "):";
                        hint = "";
                        diceAnimation.isPlaying = true;
                    } else {
                        message = "Digite o nome para " + players[currentPlayer].name + ":";
                    }
                    inputText = "";
                }
            }
            else if (currentState == "game") {
                if (!inputText.empty()) {
                    int guess = std::stoi(inputText);
                    if (guess >= 1 && guess <= maxNumber) {
                        players[currentPlayer].tries++;
                        diceAnimation.isPlaying = false;
                        
                        if (guess == targetNumber) {
                            currentPlayer++;
                            if (currentPlayer >= players.size()) {
                                currentState = "end";
                                auto minTries = std::min_element(players.begin(), players.end(),
                                    [](const Player& a, const Player& b) { return a.tries < b.tries; });
                                int minTriesValue = minTries->tries;
                                int winners = std::count_if(players.begin(), players.end(),
                                    [minTriesValue](const Player& p) { return p.tries == minTriesValue; });
                                
                                if (winners > 1) {
                                    message = "Empate! Múltiplos jogadores com " + std::to_string(minTriesValue) + " tentativas!";
                                } else {
                                    message = minTries->name + " venceu com " + std::to_string(minTriesValue) + " tentativas!";
                                }

                                for (auto& player : players) {
                                    if (scoreboard.find(player.name) == scoreboard.end()) {
                                        scoreboard[player.name] = {player.name, player.tries, 1};
                                        player.bestScore = player.tries;
                                    } else {
                                        scoreboard[player.name].gamesPlayed++;
                                        if (player.tries < scoreboard[player.name].bestScore || scoreboard[player.name].bestScore == 0) {
                                            scoreboard[player.name].bestScore = player.tries;
                                            player.bestScore = player.tries;
                                        }
                                    }
                                }
                                saveScoreboard(scoreboard);
                            } else {
                                std::random_device rd;
                                std::mt19937 gen(rd());
                                std::uniform_int_distribution<> dis(1, maxNumber);
                                targetNumber = dis(gen);
                                message = "Vez de " + players[currentPlayer].name + ". Adivinhe o número (1-" + std::to_string(maxNumber) + "):";
                                hint = "";
                                diceAnimation.isPlaying = true;
                            }
                        } else {
                            hint = guess < targetNumber ? "Tente um número maior!" : "Tente um número menor!";
                            message = "Errado! Tente novamente (1-" + std::to_string(maxNumber) + "):";
                        }
                    } else {
                        message = "Por favor, digite um número entre 1 e " + std::to_string(maxNumber) + ":";
                    }
                    inputText = "";
                }
            }
        }

        if (IsKeyPressed(KEY_TAB)) {
            showScoreboard = !showScoreboard;
        }

        int key = GetCharPressed();
        while (key > 0) {
            if (currentState == "game") {
                if ((key >= 48) && (key <= 57) && inputText.length() < 2) {
                    inputText += (char)key;
                }
            } else if (currentState == "difficulty") {
                if ((key >= 48) && (key <= 57) && inputText.length() < 1) {
                    inputText += (char)key;
                }
            } else {
                if ((key >= 32) && (key <= 125) && inputText.length() < 20) {
                    inputText += (char)key;
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (!inputText.empty()) {
                inputText.pop_back();
            }
        }

        BeginDrawing();
        ClearBackground({240, 240, 240, 255});
        
        DrawRectangleGradientV(0, 0, screenWidth, 120, {41, 128, 185, 255}, {52, 152, 219, 255});
        
        const char* title = "Jogo de Dados";
        Vector2 titleSize = MeasureTextEx(gameFont, title, 40 * titleScale, 2);
        DrawTextEx(gameFont, title, 
            {(screenWidth - titleSize.x) / 2, 40}, 
            40 * titleScale, 2, WHITE);

        Color messageColor = {0, 0, 0, (unsigned char)(255 * messageOpacity)};
        Vector2 messageSize = MeasureTextEx(gameFont, message.c_str(), 30, 2);
        DrawTextEx(gameFont, message.c_str(), 
            {(screenWidth - messageSize.x) / 2, 140}, 
            30, 2, messageColor);
        
        if (!hint.empty()) {
            Vector2 hintSize = MeasureTextEx(gameFont, hint.c_str(), 25, 2);
            DrawTextEx(gameFont, hint.c_str(), 
                {(screenWidth - hintSize.x) / 2, 180}, 
                25, 2, {231, 76, 60, 255});
        }
        
        int inputBoxWidth = 300;
        int inputBoxX = (screenWidth - inputBoxWidth) / 2;
        DrawRectangle(inputBoxX, 220, inputBoxWidth, 50, {230, 230, 230, 255});
        DrawRectangleLines(inputBoxX, 220, inputBoxWidth, 50, {200, 200, 200, 255});
        
        Vector2 inputTextSize = MeasureTextEx(gameFont, inputText.c_str(), 30, 2);
        DrawTextEx(gameFont, inputText.c_str(), 
            {inputBoxX + (inputBoxWidth - inputTextSize.x) / 2, 230}, 
            30, 2, BLACK);
        
        if (currentState == "game" || currentState == "end") {
            int playerBoxWidth = 400;
            int playerBoxX = (screenWidth - playerBoxWidth) / 2;
            int playerBoxHeight = static_cast<int>(players.size() * 50 + 20);
            DrawRectangle(playerBoxX, 290, playerBoxWidth, playerBoxHeight, {245, 245, 245, 255});
            DrawRectangleLines(playerBoxX, 290, playerBoxWidth, playerBoxHeight, {220, 220, 220, 255});
            
            for (size_t i = 0; i < players.size(); i++) {
                std::string playerInfo = players[i].name;
                if (players[i].bestScore > 0) {
                    playerInfo += " (Melhor: " + std::to_string(players[i].bestScore) + ")";
                }
                playerInfo += ": " + std::to_string(players[i].tries) + " tentativas";
                
                Color playerColor = (i == currentPlayer && currentState == "game") ? Color{231, 76, 60, 255} : Color{44, 62, 80, 255};
                Vector2 playerInfoSize = MeasureTextEx(gameFont, playerInfo.c_str(), 25, 2);
                float yPos = 300.0f + (static_cast<float>(i) * 50.0f);
                DrawTextEx(gameFont, playerInfo.c_str(), 
                    {playerBoxX + (playerBoxWidth - playerInfoSize.x) / 2, yPos}, 
                    25, 2, playerColor);
            }
        }

        if (currentState == "game" && diceAnimation.isPlaying && diceAnimation.texture.id != 0) {
            float scaledWidth = diceAnimation.frameWidth * diceAnimation.scale;
            float scaledHeight = diceAnimation.frameHeight * diceAnimation.scale;
            
            Rectangle sourceRec = {
                0,
                0,
                (float)diceAnimation.frameWidth,
                (float)diceAnimation.frameHeight
            };
            
            Rectangle destRec = {
                (screenWidth - scaledWidth) / 2.0f,
                400.0f,
                scaledWidth,
                scaledHeight
            };
            
            DrawTexturePro(diceAnimation.texture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
        }

        if (currentState == "end") {
            int buttonWidth = 200;
            int buttonHeight = 50;
            int buttonX = (screenWidth - buttonWidth) / 2;
            int buttonY = 400;
            
            Rectangle buttonRect = {
                (float)buttonX,
                (float)buttonY,
                (float)buttonWidth,
                (float)buttonHeight
            };
            
            bool isMouseOverButton = CheckCollisionPointRec(GetMousePosition(), buttonRect);
            Color buttonColor = isMouseOverButton ? Color{52, 152, 219, 255} : Color{41, 128, 185, 255};
            
            DrawRectangle(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
            DrawRectangleLines(buttonX, buttonY, buttonWidth, buttonHeight, {220, 220, 220, 255});
            
            const char* buttonText = "Jogar Novamente";
            Vector2 buttonTextSize = MeasureTextEx(gameFont, buttonText, 25, 2);
            DrawTextEx(gameFont, buttonText,
                {buttonX + (buttonWidth - buttonTextSize.x) / 2,
                 buttonY + (buttonHeight - buttonTextSize.y) / 2},
                25, 2, WHITE);
            
            if (isMouseOverButton && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                resetGame(players, currentPlayer, targetNumber, message, hint, maxNumber, currentState, diceAnimation);
            }
        }

        if (showScoreboard) {
            float scoreboardWidth = 400.0f;
            float scoreboardX = (screenWidth - scoreboardWidth) / 2.0f;
            float scoreboardHeight = static_cast<float>(scoreboard.size() * 40 + 60);
            float scoreboardY = (screenHeight - scoreboardHeight) / 2.0f;

            DrawRectangle(0, 0, screenWidth, screenHeight, {0, 0, 0, 200});
            DrawRectangle(static_cast<int>(scoreboardX), static_cast<int>(scoreboardY), 
                static_cast<int>(scoreboardWidth), static_cast<int>(scoreboardHeight), {245, 245, 245, 255});
            DrawRectangleLines(static_cast<int>(scoreboardX), static_cast<int>(scoreboardY), 
                static_cast<int>(scoreboardWidth), static_cast<int>(scoreboardHeight), {220, 220, 220, 255});

            const char* scoreboardTitle = "Placar";
            Vector2 titleSize = MeasureTextEx(gameFont, scoreboardTitle, 30, 2);
            DrawTextEx(gameFont, scoreboardTitle, 
                {scoreboardX + (scoreboardWidth - titleSize.x) / 2.0f, scoreboardY + 10.0f}, 
                30, 2, {44, 62, 80, 255});

            float y = scoreboardY + 50.0f;
            for (const auto& entry : scoreboard) {
                std::string scoreInfo = entry.second.name + " - Melhor: " + 
                    std::to_string(entry.second.bestScore) + " (Jogos: " + 
                    std::to_string(entry.second.gamesPlayed) + ")";
                DrawTextEx(gameFont, scoreInfo.c_str(), {scoreboardX + 20.0f, y}, 20, 2, {44, 62, 80, 255});
                y += 40.0f;
            }

            const char* hint = "Pressione TAB para fechar";
            Vector2 hintSize = MeasureTextEx(gameFont, hint, 20, 2);
            DrawTextEx(gameFont, hint, 
                {scoreboardX + (scoreboardWidth - hintSize.x) / 2.0f, scoreboardY + scoreboardHeight - 30.0f}, 
                20, 2, {128, 128, 128, 255});
        }
        
        EndDrawing();
    }

    UnloadTexture(diceAnimation.texture);
    CloseWindow();
    return 0;
}