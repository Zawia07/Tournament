#include <iostream>
#include <string>
#include <stdexcept> // For std::runtime_error
// #include <algorithm> // REMOVED: For std::sort on arrays, std::min
#include <limits>    // For numeric_limits
#include <fstream>   // NEW: For file operations (CSV)
#include <sstream>   // NEW: For parsing CSV lines
#include <cstdlib>   // NEW: For rand(), srand()
#include <ctime>     // NEW: For time()

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

    // Needed for sorting by MatchScheduler (used by custom sort logic)
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

// Custom Insertion Sort function for Player array (replaces std::sort)
void insertionSortPlayers(Player arr[], int n) {
    for (int i = 1; i < n; ++i) {
        Player key = arr[i];
        int j = i - 1;

        // Move elements of arr[0..i-1], that are greater than key.rank,
        // to one position ahead of their current position
        while (j >= 0 && arr[j].rank > key.rank) { // Uses Player::rank for comparison
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


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
        T* tempArray = new T[other.count]; // Dynamic array for temporary storage (C-style, not std::vector)
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

// Global constant for maximum players, consistent across modules.
const int MAX_PLAYERS_UNIVERSAL = 64; 

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
        // Replaced std::min with explicit check
        int actualDisplayCount = numToDisplay;
        if (recentMatchesLog.size() < actualDisplayCount) {
            actualDisplayCount = recentMatchesLog.size();
        }
        cout << "\n--- Recent Match Results (Last " << actualDisplayCount << " / " << recentMatchesLog.size() << " Total Recent) ---" << endl;
        
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
        insertionSortPlayers(arr, n); // Uses custom insertionSortPlayers function
    }

public:
    // Constructor now takes a reference to GameResultLogger
    MatchScheduler(GameResultLogger& logger) : numInitialPlayers(0), resultLogger(logger) {}

    // Adds a player to the tournament and initializes them for stats tracking.
    void addPlayer(const Player& player) {
        if (numInitialPlayers < MAX_PLAYERS_UNIVERSAL) {
            // Prevent adding player with duplicate ID (if IDs are read from CSV)
            // If IDs are generated sequentially, this check primarily catches external duplicates.
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
            return; // Nothing to play
        }
        
        cout << "\n--- Playing and Processing Matches ---" << endl;
        while (!scheduledMatchesQueue.isEmpty()) {
            Match currentMatch = scheduledMatchesQueue.dequeue();

            // Simulate match outcome: RANDOMLY (50/50 chance for either player)
            if (rand() % 2 == 0) { // If random number is even
                currentMatch.winner = currentMatch.player1;
            } else { // If random number is odd
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

    // Seed random number generator ONCE at the start of the program
    srand(static_cast<unsigned int>(time(0)));

    GameResultLogger gameLogger;          // Create Task 4 logger instance
    MatchScheduler scheduler(gameLogger); // Pass logger to Task 1 scheduler

    // --- CSV File Input ---
    cout << "\n--- Reading Player Data from player_ranks_full_names.csv ---" << endl;
    ifstream inputFile("player_ranks_full_names.csv");
    if (!inputFile.is_open()) {
        cerr << "Error: Could not open player_ranks_full_names.csv. Please ensure the file exists in the same directory as the executable." << endl;
        return 1; // Indicate error
    }

    string line;
    // Skip header line "full name,player name,rank"
    getline(inputFile, line); 

    int nextPlayerId = 1; // Start generating unique IDs for players
    
    // To track used ranks for uniqueness check (re-added this)
    int usedRanks[MAX_PLAYERS_UNIVERSAL];
    int numUsedRanks = 0;

    int playersReadAttempted = 0; // Count players processed from CSV (including skipped)

    while (getline(inputFile, line) && playersReadAttempted < MAX_PLAYERS_UNIVERSAL) {
        stringstream ss(line);
        string fullNameStr, playerNameStr, rankStr;

        // Attempt to parse all three fields
        if (getline(ss, fullNameStr, ',') && 
            getline(ss, playerNameStr, ',') && 
            getline(ss, rankStr)) 
        {
            playersReadAttempted++; // Increment even if player is skipped later

            try {
                int pRank = stoi(rankStr);
                
                // --- Input Validation & Uniqueness Checks ---
                // Basic validation for parsed data
                if (playerNameStr.empty()) {
                     cerr << "Warning: Player name is empty in line: '" << line << "'. Using 'Unnamed Player'." << endl;
                     playerNameStr = "Unnamed Player";
                }
                if (pRank < 0) {
                    cerr << "Warning: Invalid Rank (Rank < 0) for player '" << playerNameStr << "' in line: '" << line << "'. Skipping." << endl;
                    continue; // Skip this player
                }
                
                // Check for rank uniqueness against already successfully added players
                bool rankAlreadyUsed = false;
                for (int j = 0; j < numUsedRanks; ++j) {
                    if (usedRanks[j] == pRank) {
                        cerr << "Warning: Rank " << pRank << " is duplicated for player '" << playerNameStr << "' in line: '" << line << "'. This player will be skipped." << endl;
                        rankAlreadyUsed = true;
                        break;
                    }
                }
                if (rankAlreadyUsed) {
                    continue; // Skip this player
                }
                
                // If all checks pass, create Player object, add to scheduler, and mark rank as used
                Player newPlayer(nextPlayerId++, playerNameStr, pRank); // Assign new sequential ID
                scheduler.addPlayer(newPlayer); // This method also handles duplicate IDs for robustness
                usedRanks[numUsedRanks++] = pRank;

            } catch (const invalid_argument& e) {
                cerr << "Warning: Invalid number format for Rank in line: '" << line << "' - " << e.what() << ". Skipping." << endl;
            } catch (const out_of_range& e) {
                cerr << "Warning: Number out of range for Rank in line: '" << line << "' - " << e.what() << ". Skipping." << endl;
            }
        } else {
            // Check if it's not just an empty line at the end of file
            if (!line.empty()) {
                 cerr << "Warning: Malformed line (expected 'full name,player name,rank'): '" << line << "'. Skipping." << endl;
            }
        }
    }
    inputFile.close();

    int actualRegisteredPlayers = scheduler.getNumInitialPlayers();
    if (actualRegisteredPlayers == 0) {
        cout << "\nNo players were successfully loaded from the CSV file. Please check 'player_ranks_full_names.csv' format and content, and ensure no duplicate ranks." << endl;
        return 0;
    }
    cout << "Successfully loaded and registered " << actualRegisteredPlayers << " players from CSV." << endl;
    
    // Initialize tournament with all successfully added players
    scheduler.initializeTournament(); 

    if (actualRegisteredPlayers == 1) {
        cout << "\nOnly one player registered. This player is the champion by default." << endl;
    }

    // --- Tournament Simulation Loop ---
    cout << "\n===== TOURNAMENT SIMULATION BEGINS =====" << endl;
    int roundNum = 1;
    while (true) {
        if (scheduler.isTournamentOver()) {
            break; // Tournament ends if only one player is left
        }
        cout << "\n<<<<< ROUND " << roundNum << " >>>>>" << endl;

        if (!scheduler.createNextRoundPairings()) {
            // This implies not enough players were in waiting queue to form new matches.
            // isTournamentOver() should have caught the 1-player case.
            if (scheduler.isTournamentOver()) break; 
            break;
        }
        scheduler.playAndProcessMatches();   // Simulate matches, update winners

        if (!scheduler.advanceToNextRound()) {
            // If advanceToNextRound returns false, it often means the tournament is over 
            if (scheduler.isTournamentOver()) break; 
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