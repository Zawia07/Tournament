#include <iostream>
#include <string>
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::sort on arrays, std::min
#include <limits>    // For numeric_limits
// NO <vector>, <list> or other STL containers explicitly disallowed.

using namespace std;

// --- Player Struct (from Task 1) ---
struct Player {
    int id;
    string name;
    int rank; // Lower is better

    Player(int i = 0, string n = "N/A", int r = 9999) : id(i), name(n), rank(r) {}

    friend ostream& operator<<(ostream& os, const Player& p) {
        os << "ID: " << p.id << ", Name: " << p.name << ", Rank: " << p.rank;
        return os;
    }

    // Needed for sorting by MatchScheduler
    bool operator<(const Player& other) const {
        return rank < other.rank;
    }
    // Optional: equality operators if needed elsewhere
    bool operator==(const Player& other) const {
        return id == other.id;
    }
    bool operator!=(const Player& other) const {
        return !(*this == other);
    }
};

// --- Match Struct (from Task 1) ---
struct Match {
    static int nextMatchIdCounter; // Static counter for unique match IDs
    int matchId;
    Player player1;
    Player player2;
    Player winner; // Winner player object
    bool played;   // Flag to indicate if the match has been played

    Match(Player p1 = Player(), Player p2 = Player()) : player1(p1), player2(p2), played(false) {
        matchId = nextMatchIdCounter++;
        // 'winner' is a default Player() (ID 0) until determined
    }

    friend ostream& operator<<(ostream& os, const Match& m) {
        os << "Match ID: " << m.matchId << " | " << m.player1.name << " vs " << m.player2.name;
        if (m.played) {
            if (m.winner.id != 0) { // Check if winner is a valid player
                os << " | Winner: " << m.winner.name;
            } else {
                os << " | Winner: Undecided/Draw"; // Or if ID 0 is never a valid player ID
            }
        } else {
            os << " | Status: Pending";
        }
        return os;
    }
};

int Match::nextMatchIdCounter = 1; // Initialize static member for Match IDs

// --- CustomQueue Class Template (from Task 1, assumed complete) ---
template <typename T>
class CustomQueue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(val), next(nullptr) {}
    };

    Node* frontNode;
    Node* rearNode;
    int count;

public:
    CustomQueue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    ~CustomQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void enqueue(const T& item) {
        Node* newNode = new Node(item);
        if (isEmpty()) {
            frontNode = rearNode = newNode;
        }
        else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        count++;
    }

    T dequeue() {
        if (isEmpty()) {
            throw runtime_error("Queue is empty, cannot dequeue.");
        }
        Node* temp = frontNode;
        T data = temp->data;
        frontNode = frontNode->next;
        if (frontNode == nullptr) {
            rearNode = nullptr;
        }
        delete temp;
        count--;
        return data;
    }

    T peek() const {
        if (isEmpty()) {
            throw runtime_error("Queue is empty, cannot peek.");
        }
        return frontNode->data;
    }

    CustomQueue(const CustomQueue& other) : frontNode(nullptr), rearNode(nullptr), count(0) {
        Node* current = other.frontNode;
        while (current != nullptr) {
            this->enqueue(current->data);
            current = current->next;
        }
    }

    CustomQueue& operator=(const CustomQueue& other) {
        if (this == &other) {
            return *this;
        }
        while (!isEmpty()) {
            dequeue();
        }
        Node* current = other.frontNode;
        while (current != nullptr) {
            this->enqueue(current->data);
            current = current->next;
        }
        return *this;
    }
};


// =======================================================================================
// --- TASK 4: GAME RESULT LOGGING & PERFORMANCE HISTORY ---
// =======================================================================================

// --- CustomStack Class Template (NEW for Task 4) ---
template <typename T>
class CustomStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(val), next(nullptr) {}
    };

    Node* topNode;
    int count;

public:
    CustomStack() : topNode(nullptr), count(0) {}

    ~CustomStack() {
        while (!isEmpty()) {
            pop();
        }
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void push(const T& item) {
        Node* newNode = new Node(item);
        newNode->next = topNode;
        topNode = newNode;
        count++;
    }

    T pop() {
        if (isEmpty()) {
            throw runtime_error("Stack is empty, cannot pop.");
        }
        Node* temp = topNode;
        T data = temp->data;
        topNode = topNode->next;
        delete temp;
        count--;
        return data;
    }

    T peek() const {
        if (isEmpty()) {
            throw runtime_error("Stack is empty, cannot peek.");
        }
        return topNode->data;
    }

    CustomStack(const CustomStack& other) : topNode(nullptr), count(0) {
        if (other.topNode == nullptr) {
            return;
        }
        // To copy a stack and maintain order, elements from the 'other' stack
        // are put into a temporary array, then pushed onto 'this' stack in reverse
        // order of how they were read from 'other'.
        Node* current = other.topNode;
        T* tempArray = new T[other.count]; // Dynamic array for temporary storage
        int i = 0;
        while(current != nullptr) {
            tempArray[i++] = current->data; // tempArray[0] is top of 'other'
            current = current->next;
        }
        // Push elements from the end of tempArray first to maintain stack order
        for (int j = i - 1; j >= 0; --j) {
            this->push(tempArray[j]);
        }
        delete[] tempArray; // Clean up temporary dynamic array
    }

    CustomStack& operator=(const CustomStack& other) {
        if (this == &other) {
            return *this;
        }
        // Clear current stack
        while (!isEmpty()) {
            pop();
        }
        // Copy from other (similar logic to copy constructor)
        if (other.topNode == nullptr) {
            return *this;
        }
        Node* current = other.topNode;
        T* tempArray = new T[other.count];
        int i = 0;
        while(current != nullptr) {
            tempArray[i++] = current->data;
            current = current->next;
        }
        for (int j = i - 1; j >= 0; --j) {
            this->push(tempArray[j]);
        }
        delete[] tempArray;
        return *this;
    }
};

// --- HistoricalMatch Struct (NEW for Task 4) ---
// Used to store a snapshot of match details for logging purposes.
struct HistoricalMatch {
    int matchId;
    int player1Id;
    string player1Name;
    int player2Id;
    string player2Name;
    int winnerId;
    string winnerName;
    // Future enhancements could include: stage (qualifier, final), score, game-specific stats.

    HistoricalMatch(int mId = 0, int p1Id = 0, string p1N = "N/A",
                    int p2Id = 0, string p2N = "N/A",
                    int wId = 0, string wN = "N/A")
        : matchId(mId), player1Id(p1Id), player1Name(p1N),
          player2Id(p2Id), player2Name(p2N),
          winnerId(wId), winnerName(wN) {}

    friend ostream& operator<<(ostream& os, const HistoricalMatch& hm) {
        os << "Match ID: " << hm.matchId << " | P1: " << hm.player1Name << " (ID:" << hm.player1Id << ")"
           << " vs P2: " << hm.player2Name << " (ID:" << hm.player2Id << ")";
        if (hm.winnerId != 0) { // Assuming player ID 0 is not a valid playing ID
            os << " | Winner: " << hm.winnerName << " (ID:" << hm.winnerId << ")";
        } else {
            os << " | Winner: TBD / Draw / No valid winner";
        }
        return os;
    }
};

// --- PlayerStats Struct (NEW for Task 4) ---
// Tracks performance statistics for a single player.
struct PlayerStats {
    int playerId;
    string playerName;
    int initialRank; // Store player's rank at the start of the tournament
    int wins;
    int losses;

    PlayerStats(int id = 0, string name = "N/A", int r = 9999)
        : playerId(id), playerName(name), initialRank(r), wins(0), losses(0) {}

    void recordWin() { wins++; }
    void recordLoss() { losses++; }
    int totalMatchesPlayed() const { return wins + losses; }

    friend ostream& operator<<(ostream& os, const PlayerStats& ps) {
        os << "Player: " << ps.playerName << " (ID: " << ps.playerId << ", Initial Rank: " << ps.initialRank << ") | "
           << "Wins: " << ps.wins << ", Losses: " << ps.losses
           << ", Matches Played: " << ps.totalMatchesPlayed();
        return os;
    }
};

// Global constant for maximum players, consistent across modules.
const int MAX_PLAYERS_UNIVERSAL = 64; 

// --- GameResultLogger Class (NEW for Task 4) ---
// Manages logging of match results and tracking player performance.
class GameResultLogger {
private:
    CustomStack<HistoricalMatch> recentMatchesLog;          // Stack for recent match results (LIFO)
    CustomQueue<HistoricalMatch> allMatchesChronologicalLog; // Queue for all matches in order (FIFO)

    PlayerStats playerStatsArray[MAX_PLAYERS_UNIVERSAL]; // Array to store stats for each player
    int numTrackedPlayersInStats;                        // Count of players currently in playerStatsArray

    // Helper to find a player's index in the stats array. Returns -1 if not found.
    int findPlayerStatsArrayIndex(int playerId) const {
        for (int i = 0; i < numTrackedPlayersInStats; ++i) {
            if (playerStatsArray[i].playerId == playerId) {
                return i;
            }
        }
        return -1; 
    }

public:
    GameResultLogger() : numTrackedPlayersInStats(0) {}

    // Initializes a stats entry for a new player.
    void initializePlayerForStats(const Player& player) {
        if (numTrackedPlayersInStats < MAX_PLAYERS_UNIVERSAL) {
            if (findPlayerStatsArrayIndex(player.id) == -1) { // Add only if not already present
                playerStatsArray[numTrackedPlayersInStats++] = PlayerStats(player.id, player.name, player.rank);
            }
        } else {
            cerr << "Warning: Player stats array is full. Cannot add stats entry for " << player.name << "." << endl;
        }
    }

    // Records the outcome of a completed match.
    void recordMatchOutcome(const Match& completedMatch) {
        if (!completedMatch.played) {
            // Typically, only played matches with definite outcomes are logged for stats.
            // Byes are handled by advancing players directly, not as a "played match" for this log.
            return;
        }
        // Ensure winner is valid (not default Player object with ID 0)
        if (completedMatch.winner.id == 0 || completedMatch.winner.name == "N/A") {
            cerr << "Warning: Match (ID: " << completedMatch.matchId 
                 << ") outcome is unclear (invalid winner). Performance log update skipped." << endl;
            return; 
        }

        HistoricalMatch histMatch(
            completedMatch.matchId,
            completedMatch.player1.id, completedMatch.player1.name,
            completedMatch.player2.id, completedMatch.player2.name,
            completedMatch.winner.id, completedMatch.winner.name
        );

        recentMatchesLog.push(histMatch);             // Add to recent matches stack
        allMatchesChronologicalLog.enqueue(histMatch); // Add to chronological log queue

        // Update win/loss stats for both participating players
        int p1Idx = findPlayerStatsArrayIndex(completedMatch.player1.id);
        int p2Idx = findPlayerStatsArrayIndex(completedMatch.player2.id);

        if (p1Idx != -1) {
            if (completedMatch.winner.id == completedMatch.player1.id) {
                playerStatsArray[p1Idx].recordWin();
            } else { // If P1 is not winner, P1 is loser (assuming no draws for stat updates)
                playerStatsArray[p1Idx].recordLoss();
            }
        } else {
            cerr << "Warning: Player " << completedMatch.player1.name << " (ID: " << completedMatch.player1.id
                 << ") not found in stats tracking. Performance not updated." << endl;
        }

        if (p2Idx != -1) {
            if (completedMatch.winner.id == completedMatch.player2.id) {
                playerStatsArray[p2Idx].recordWin();
            } else { // If P2 is not winner, P2 is loser
                playerStatsArray[p2Idx].recordLoss();
            }
        } else {
             cerr << "Warning: Player " << completedMatch.player2.name << " (ID: " << completedMatch.player2.id
                 << ") not found in stats tracking. Performance not updated." << endl;
        }
    }

    // Key Action: Store recent match results for quick review.
    void displayRecentMatches(int numToDisplay = 5) const {
        cout << "\n--- Recent Match Results (Last " << min(numToDisplay, recentMatchesLog.size()) << " / " << recentMatchesLog.size() << " Total Recent) ---" << endl;
        if (recentMatchesLog.isEmpty()) {
            cout << "No match results have been recorded yet." << endl;
        } else {
            CustomStack<HistoricalMatch> tempStack = recentMatchesLog; // Use copy for non-destructive display
            int displayedCount = 0;
            while (!tempStack.isEmpty() && displayedCount < numToDisplay) {
                cout << tempStack.pop() << endl; // Pop shows most recent first
                displayedCount++;
            }
        }
        cout << "--------------------------------------------------------------------" << endl;
    }

    // Key Action: Maintain a structured history / Provide easy access to past results.
    void displayAllRecordedMatches() const {
        cout << "\n--- All Recorded Matches (" << allMatchesChronologicalLog.size() << " Total) ---" << endl;
        if (allMatchesChronologicalLog.isEmpty()) {
            cout << "No matches have been recorded in the chronological log." << endl;
        } else {
            CustomQueue<HistoricalMatch> tempQueue = allMatchesChronologicalLog; // Use copy
            int counter = 1;
            while (!tempQueue.isEmpty()) {
                cout << counter++ << ". " << tempQueue.dequeue() << endl;
            }
        }
        cout << "--------------------------------------------------------------------" << endl;
    }

    // Key Action: Player performance tracking / Easy access to past results for tournament analysis.
    void displaySinglePlayerPerformance(int playerId) const {
        cout << "\n--- Full Performance Record for Player ID: " << playerId << " ---" << endl;
        int playerIdx = findPlayerStatsArrayIndex(playerId);
        if (playerIdx != -1) {
            cout << playerStatsArray[playerIdx] << endl; // Display summary stats
            
            // List all matches involving this player from the chronological log
            cout << "  Matches involving " << playerStatsArray[playerIdx].playerName << ":" << endl;
            CustomQueue<HistoricalMatch> tempAllMatches = allMatchesChronologicalLog; // Copy
            bool foundPlayerMatch = false;
            int matchCounter = 1;
            while(!tempAllMatches.isEmpty()){
                HistoricalMatch currentMatch = tempAllMatches.dequeue();
                if(currentMatch.player1Id == playerId || currentMatch.player2Id == playerId) {
                    cout << "    " << matchCounter++ << ". " << currentMatch << endl;
                    foundPlayerMatch = true;
                }
            }
            if(!foundPlayerMatch) {
                cout << "    No specific match records found in the log for this player." << endl;
            }
        } else {
            cout << "Player with ID " << playerId << " not found or no stats recorded for them." << endl;
        }
        cout << "--------------------------------------------------------------------" << endl;
    }

    void displayAllPlayersPerformanceSummary() const {
        cout << "\n--- All Player Performance Summaries (" << numTrackedPlayersInStats << " Players Tracked) ---" << endl;
        if (numTrackedPlayersInStats == 0) {
            cout << "No player performance data available." << endl;
        } else {
            for (int i = 0; i < numTrackedPlayersInStats; ++i) {
                cout << playerStatsArray[i] << endl;
            }
        }
        cout << "--------------------------------------------------------------------" << endl;
    }
};
// =======================================================================================
// --- END OF TASK 4 ---
// =======================================================================================


// --- MatchScheduler Class (MODIFIED from Task 1 to integrate GameResultLogger) ---
class MatchScheduler {
private:
    Player initialPlayers[MAX_PLAYERS_UNIVERSAL]; // Array to store all initially added players
    int numInitialPlayers;                        // Count of players in initialPlayers

    CustomQueue<Player> waitingPlayersQueue; // Players waiting for their next match
    CustomQueue<Match> scheduledMatchesQueue; // Matches scheduled but not yet played
    CustomQueue<Player> winnersQueue;         // Winners from the last round of played matches
    
    GameResultLogger& resultLogger; // Reference to the GameResultLogger instance (NEW)

    // Helper to sort players by rank (lower rank is better)
    void sortPlayersByRank(Player arr[], int n) {
        sort(arr, arr + n); // Uses std::sort from <algorithm>
    }

public:
    // Constructor now takes a reference to GameResultLogger
    MatchScheduler(GameResultLogger& logger) : numInitialPlayers(0), resultLogger(logger) {}

    // Adds a player to the tournament and initializes them for stats tracking.
    void addPlayer(const Player& player) {
        if (numInitialPlayers < MAX_PLAYERS_UNIVERSAL) {
            // Prevent adding player with duplicate ID
            for(int i=0; i < numInitialPlayers; ++i) {
                if(initialPlayers[i].id == player.id) {
                    cerr << "Error: Player with ID " << player.id << " (" << initialPlayers[i].name 
                         << ") already exists. Cannot add " << player.name << " with the same ID." << endl;
                    return; // Skip adding this player
                }
            }
            initialPlayers[numInitialPlayers++] = player;
            resultLogger.initializePlayerForStats(player); // NEW: Initialize player in logger
        } else {
            cerr << "Error: Cannot add more players. Maximum capacity (" << MAX_PLAYERS_UNIVERSAL << ") reached." << endl;
        }
    }
    
    int getNumInitialPlayers() const { // Helper to get current count of registered players
        return numInitialPlayers;
    }

    // Initializes the tournament by seeding players into the waiting queue.
    // By default (numPlayersToSeed = 0), all registered players are seeded.
    void initializeTournament(int numPlayersToSeed = 0) {
        if (numInitialPlayers == 0) {
            cout << "No players added to the tournament to initialize." << endl;
            return;
        }

        // Determine how many players to seed: specified count or all initial players
        int countToSeed = (numPlayersToSeed > 0 && numPlayersToSeed <= numInitialPlayers) ? numPlayersToSeed : numInitialPlayers;

        // Sort the players to be seeded by rank
        sortPlayersByRank(initialPlayers, countToSeed); 

        cout << "\n--- Initial Seeding (" 
             << ((countToSeed == numInitialPlayers && numPlayersToSeed == 0) ? "All " : "Top ") 
             << countToSeed << " Players by Rank) ---" << endl;
        for (int i = 0; i < countToSeed; ++i) {
            waitingPlayersQueue.enqueue(initialPlayers[i]);
            cout << (i + 1) << ". " << initialPlayers[i].name << " (Rank: " << initialPlayers[i].rank << ")" << endl;
        }
        cout << "------------------------------------" << endl;

        if (waitingPlayersQueue.size() > 1 && waitingPlayersQueue.size() % 2 != 0) {
            cout << "Notification: Odd number of players (" << waitingPlayersQueue.size() 
                 << ") in the first round. One player will receive a bye." << endl;
        }
    }

    // Creates match pairings for the next round from players in waitingPlayersQueue.
    bool createNextRoundPairings() {
        if (waitingPlayersQueue.size() < 2) {
            // Not enough players to form a match.
            // If 1 player is waiting, they might be the overall winner or have received a bye.
            return false; 
        }

        cout << "\n--- Creating Pairings for Next Round ---" << endl;

        // Extract players from waiting queue to a temporary array for pairing logic
        Player roundPlayersArray[MAX_PLAYERS_UNIVERSAL];
        int numRoundPlayers = 0;
        while(!waitingPlayersQueue.isEmpty() && numRoundPlayers < MAX_PLAYERS_UNIVERSAL) {
            roundPlayersArray[numRoundPlayers++] = waitingPlayersQueue.dequeue();
        }
        
        // Pairing strategy: e.g., first vs last, second vs second-to-last, etc.
        // This assumes roundPlayersArray is already sorted if a specific seeded pairing is desired.
        // The initial sort in initializeTournament helps; subsequent rounds depend on winner order.
        int i = 0, j = numRoundPlayers - 1;
        while (i < j) {
            Match newMatch(roundPlayersArray[i], roundPlayersArray[j]);
            scheduledMatchesQueue.enqueue(newMatch);
            cout << "Scheduled: " << newMatch.player1.name << " vs " << newMatch.player2.name << endl;
            i++;
            j--;
        }

        if (i == j) { // One player remains in the middle - receives a bye
            Player byePlayer = roundPlayersArray[i];
            cout << byePlayer.name << " gets a BYE and advances directly to the winners' pool." << endl;
            winnersQueue.enqueue(byePlayer); 
        }
        cout << "------------------------------------" << endl;
        // Return true if any matches were scheduled OR a bye was processed
        return !scheduledMatchesQueue.isEmpty() || (numRoundPlayers > 0 && i == j);
    }

    // Simulates playing scheduled matches and processes their outcomes.
    void playAndProcessMatches() {
        // Check if there are any matches to play
        if (scheduledMatchesQueue.isEmpty()) {
            // This is normal if, for example, only byes were processed in createNextRoundPairings
            // or if the tournament is nearing its end.
            // cout << "\nNo actual matches were scheduled in this phase." << endl; 
            return; // Nothing to play
        }
        
        cout << "\n--- Playing and Processing Matches ---" << endl;
        while (!scheduledMatchesQueue.isEmpty()) {
            Match currentMatch = scheduledMatchesQueue.dequeue();

            // Simulate match outcome: player with lower rank number wins.
            // If ranks are equal, player1 wins (arbitrary but consistent tie-break).
            if (currentMatch.player1.rank <= currentMatch.player2.rank) {
                currentMatch.winner = currentMatch.player1;
            } else {
                currentMatch.winner = currentMatch.player2;
            }
            currentMatch.played = true;

            cout << currentMatch.player1.name << " vs " << currentMatch.player2.name
                 << " -> Winner: " << currentMatch.winner.name << endl;
            
            resultLogger.recordMatchOutcome(currentMatch); // NEW: Record match outcome for Task 4

            winnersQueue.enqueue(currentMatch.winner); // Add winner to the winners' queue
        }
        cout << "------------------------------------" << endl;
    }

    // Advances winners from the winnersQueue to the waitingPlayersQueue for the next round.
    bool advanceToNextRound() {
        // First, ensure any players left in waitingPlayersQueue (e.g. due to an interrupted process)
        // are moved to winnersQueue if they were meant to advance (e.g. byes not yet moved).
        // However, createNextRoundPairings should clear waitingPlayersQueue or handle byes directly to winnersQueue.
        // This step ensures all "advancers" (winners of matches or byes) are in winnersQueue.

        if (winnersQueue.isEmpty() && waitingPlayersQueue.isEmpty()) {
             // No one won, no one is waiting. Tournament must be over or had no players.
            return false;
        }

        // If only one "winner" remains and no other queues have players (except potentially waiting for this final move)
        // this player is the champion. Move them to waiting queue for the final check.
        if (winnersQueue.size() == 1 && waitingPlayersQueue.isEmpty() && scheduledMatchesQueue.isEmpty()) {
            Player finalWinnerCandidate = winnersQueue.peek(); 
            waitingPlayersQueue.enqueue(winnersQueue.dequeue()); // Move to waiting for getTournamentWinner
            cout << "\n--- Advancing Final Player ---" << endl;
            cout << finalWinnerCandidate.name << " is the sole remaining player." << endl;
            cout << "------------------------------------" << endl;
            return false; // Signals that this is likely the conclusion of rounds.
        }
        
        cout << "\n--- Advancing Winners to Next Round's Waiting Pool ---" << endl;
        while (!winnersQueue.isEmpty()) {
            Player winnerToAdvance = winnersQueue.dequeue();
            waitingPlayersQueue.enqueue(winnerToAdvance);
            cout << winnerToAdvance.name << " advances." << endl;
        }
        cout << "------------------------------------" << endl;

        // Continue tournament if there are players in the waiting queue for the next round.
        // If waitingPlayersQueue.size() is 1 now, isTournamentOver() will catch it.
        return waitingPlayersQueue.size() >= 1; 
    }

    // Placeholder for display functions from original Task 1 code, if any were present.
    // User can re-insert their specific implementations for these.
    void displayScheduledMatches() const { 
        cout << "\n--- Current Scheduled Matches (" << scheduledMatchesQueue.size() << ") ---" << endl;
        if (scheduledMatchesQueue.isEmpty()) {
            cout << "No matches currently scheduled." << endl;
        } else {
            CustomQueue<Match> tempQueue = scheduledMatchesQueue; // Use copy
            while (!tempQueue.isEmpty()) {
                cout << tempQueue.dequeue() << endl;
            }
        }
        cout << "------------------------------------" << endl;
    }
    void displayWaitingPlayers() const { 
        cout << "\n--- Players Waiting for Next Round (" << waitingPlayersQueue.size() << ") ---" << endl;
        if (waitingPlayersQueue.isEmpty()) {
            cout << "No players currently waiting." << endl;
        } else {
            CustomQueue<Player> tempQueue = waitingPlayersQueue; // Use copy
            while (!tempQueue.isEmpty()) {
                cout << tempQueue.dequeue() << endl;
            }
        }
        cout << "------------------------------------" << endl;    
    }
    void displayWinners() const { 
        cout << "\n--- Winners from Last Played Round (Before Advancing) (" << winnersQueue.size() << ") ---" << endl;
        if (winnersQueue.isEmpty()) {
            cout << "No winners recorded from the last round / No matches played yet / Winners already advanced." << endl;
        } else {
            CustomQueue<Player> tempQueue = winnersQueue; // Use copy
            while (!tempQueue.isEmpty()) {
                cout << tempQueue.dequeue() << endl;
            }
        }
        cout << "------------------------------------" << endl;
    }


    // Checks if the tournament has concluded (i.e., only one player left in waiting, no pending matches).
    bool isTournamentOver() const {
        return (waitingPlayersQueue.size() == 1 && scheduledMatchesQueue.isEmpty() && winnersQueue.isEmpty());
    }

    // Returns the tournament champion if the tournament is over.
    Player getTournamentWinner() const {
        if (isTournamentOver()) {
            return waitingPlayersQueue.peek(); // The sole player in waiting queue is the winner
        }
        // Fallback for an unlikely state where winner is somehow left in winnersQueue as the sole player
        // and other queues are empty. isTournamentOver should ideally catch this.
        if (winnersQueue.size() == 1 && waitingPlayersQueue.isEmpty() && scheduledMatchesQueue.isEmpty()) {
            return winnersQueue.peek();
        }
        return Player(); // Returns a default Player (ID 0, Name "N/A") if no winner yet or error state
    }
};


// --- Main Function (MODIFIED from Task 1 to include Task 4 and refined input) ---
int main() {
    cout << "===== APU Esports Championship Management System =====" << endl;

    GameResultLogger gameLogger;          // Create Task 4 logger instance
    MatchScheduler scheduler(gameLogger); // Pass logger to Task 1 scheduler

    int numPlayersInput; // Number of players user wants to register

    // Get number of players to register
    while (true) {
        cout << "\nEnter the number of players to register (1-" << MAX_PLAYERS_UNIVERSAL << "): ";
        cin >> numPlayersInput;
        if (cin.fail() || numPlayersInput < 1 || numPlayersInput > MAX_PLAYERS_UNIVERSAL) {
            cin.clear(); // Clear error flags
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            cout << "Invalid input. Please enter a number between 1 and " << MAX_PLAYERS_UNIVERSAL << "." << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard the rest of the line
            break;
        }
    }

    // Array to keep track of ranks used, to ensure uniqueness
    int usedRanks[MAX_PLAYERS_UNIVERSAL];
    int numUsedRanks = 0;
    int nextPlayerId = 1; // Assign unique player IDs starting from 1

    cout << "\n--- Player Registration ---" << endl;
    for (int i = 0; i < numPlayersInput; ++i) {
        string pName;
        int pRank;
        bool rankIsUnique;
        int pId = nextPlayerId++; // Assign a new unique ID

        cout << "\nEnter details for Player " << (i + 1) << " (System ID: " << pId << "):" << endl;
        
        // Get player name
        cout << "Name: ";
        getline(cin, pName); 
        if (pName.empty()) { // Basic validation for name
            cout << "Player name cannot be empty. Using default 'Player " << pId << "'." << endl;
            pName = "Player " + to_string(pId);
        }

        // Get player rank and ensure it's unique
        do {
            rankIsUnique = true;
            cout << "Rank (integer, lower is better, must be unique): ";
            cin >> pRank;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid rank format. Please enter an integer." << endl;
                rankIsUnique = false;
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard rest of the line
            
            // Check for rank uniqueness
            for (int j = 0; j < numUsedRanks; ++j) {
                if (usedRanks[j] == pRank) {
                    cout << "Rank " << pRank << " is already taken by another player. Please enter a unique rank." << endl;
                    rankIsUnique = false;
                    break;
                }
            }
        } while (!rankIsUnique);
        usedRanks[numUsedRanks++] = pRank; // Add new rank to used list
        
        scheduler.addPlayer(Player(pId, pName, pRank)); // Add player to scheduler (and logger)
    }
    
    int actualRegisteredPlayers = scheduler.getNumInitialPlayers();
    if (actualRegisteredPlayers == 0) {
        cout << "\nNo players were successfully registered. Exiting tournament." << endl;
        return 0;
    }
    
    scheduler.initializeTournament(); // Initialize with all successfully added players

    if (actualRegisteredPlayers == 1) {
        cout << "\nOnly one player registered. This player is the champion by default." << endl;
        // The tournament loop below will correctly identify this.
    }

    // --- Tournament Simulation Loop ---
    cout << "\n===== TOURNAMENT SIMULATION BEGINS =====" << endl;
    int roundNum = 1;
    while (true) {
        if (scheduler.isTournamentOver()) {
            break; // Tournament ends if only one player is left
        }
        cout << "\n<<<<< ROUND " << roundNum << " >>>>>" << endl;
        // scheduler.displayWaitingPlayers(); // Optional: Display players waiting for this round

        if (!scheduler.createNextRoundPairings()) {
            // This implies not enough players were in waiting queue to form new matches.
            // isTournamentOver() should have caught the 1-player case.
            // If 0 players, or some other issue, break.
            if (scheduler.isTournamentOver()) break; // Double check
            // cout << "Not enough players to create further pairings or tournament has concluded." << endl; // Verbose
            break;
        }
        // scheduler.displayScheduledMatches(); // Optional: Display newly scheduled matches
        scheduler.playAndProcessMatches();   // Simulate matches, update winners
        // scheduler.displayWinners();        // Optional: Display winners of this round

        if (!scheduler.advanceToNextRound()) {
            // If advanceToNextRound returns false, it often means the tournament is over 
            // (e.g., only one winner was advanced, now sole player in waitingPlayersQueue).
            if (scheduler.isTournamentOver()) break; // Final check after advancement attempt
            // cout << "Advancement phase indicates no more rounds or an issue." << endl; // Verbose
            break; 
        }
        roundNum++;
        
        // Safety break for very long tournaments, indicating a possible loop or error
        if (roundNum > actualRegisteredPlayers * 2 && actualRegisteredPlayers > 1) { 
            cout << "Warning: Tournament simulation is taking unusually long. Halting to prevent infinite loop." << endl;
            break;
        }
    }

    cout << "\n===== TOURNAMENT SIMULATION COMPLETE =====" << endl;
    Player champion = scheduler.getTournamentWinner();
    if (champion.id != 0 && champion.name != "N/A") { // Check if a valid winner was returned
        cout << "Champion: " << champion.name << " (ID: " << champion.id << ", Rank: " << champion.rank << ")" << endl;
    } else if (actualRegisteredPlayers == 1 && scheduler.getNumInitialPlayers() == 1) {
        // This case should ideally be handled by getTournamentWinner() if logic is perfect.
        // However, as a fallback for a single-player tournament:
        cout << "The sole registered player is the champion by default." << endl;
    }
    else {
        cout << "No single champion determined, or the tournament ended prematurely." << endl;
        // Optional: Display final state of queues if needed for debugging
        // scheduler.displayWaitingPlayers();
        // scheduler.displayWinners();
    }

    // --- Task 4: Post-Tournament Reports Menu ---
    cout << "\n===== POST-TOURNAMENT REPORTS (TASK 4) =====" << endl;
    char reportChoice;
    do {
        cout << "\nGame Result Logging & Performance History Options:" << endl;
        cout << "1. Display Recent Match Results" << endl;
        cout << "2. Display All Logged Matches (Chronological)" << endl;
        cout << "3. Display All Player Performances Summary" << endl;
        cout << "4. Display Specific Player's Full Performance Record" << endl;
        cout << "X. Exit Reports Menu" << endl;
        cout << "Enter your choice: ";
        cin >> reportChoice;
        // Clear buffer, especially after reading a char then expecting getline or other inputs
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

        switch (toupper(reportChoice)) {
            case '1': {
                int count;
                cout << "How many recent matches to display? (e.g., 5): ";
                cin >> count;
                if (cin.fail() || count <= 0) {
                    cout << "Invalid count entered. Defaulting to display up to 5 recent matches." << endl;
                    count = 5; // Default value
                    cin.clear(); // Clear error flags if cin failed
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Always clear buffer after cin >>
                gameLogger.displayRecentMatches(count);
                break;
            }
            case '2':
                gameLogger.displayAllRecordedMatches();
                break;
            case '3':
                gameLogger.displayAllPlayersPerformanceSummary();
                break;
            case '4': {
                int playerIdToView;
                cout << "Enter Player ID to display their full performance record: ";
                cin >> playerIdToView;
                if (cin.fail()) {
                    cout << "Invalid Player ID format. Please enter an integer." << endl;
                    cin.clear(); 
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if(!cin.fail()) { // Proceed only if input was valid
                    gameLogger.displaySinglePlayerPerformance(playerIdToView);
                }
                break;
            }
            case 'X':
                cout << "Exiting reports menu." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (toupper(reportChoice) != 'X');

    cout << "\nExiting Esports Championship Management System. Goodbye!" << endl;
    return 0;
}