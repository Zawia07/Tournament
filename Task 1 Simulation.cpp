#include <iostream>
#include <string>
#include <stdexcept> // For std::runtime_error
#include <limits>    // For numeric_limits
#include <fstream>   // For file operations (CSV)
#include <sstream>   // For parsing CSV lines
#include <cstdlib>   // For rand(), srand()
#include <ctime>     // For time()


using namespace std;

// Global constant for maximum players, consistent across modules.
const int MAX_PLAYERS_UNIVERSAL = 64; 

// --- Player Struct (Consolidated from both systems) ---
// This structure now includes fields for both tournament play and registration management.
struct Player {
    string playerId; // Changed from int id to string
    string playerName;
    int ranking;
    string registrationStatus; // Current status: "Early birds", "Regular", "Wildcard", "Checked In", "Withdrawn", "Replacement"
    string registrationTime;   // For tracking time (not extensively used in this combined logic, but kept)
    string originalPriority;   // The type of registration initially from CSV (e.g., "early birds", "regular", "wildcard")

    Player() : 
        playerId(""), 
        playerName(""), 
        ranking(0), 
        registrationStatus(""), 
        registrationTime(""),
        originalPriority(""){}

    // Constructor to match CSV loading and replacement player creation
    // `status` here refers to the initial "early birds", "regular", "wildcard" for file load,
    // or the "Replacement" status for replacement players.
    // `originalPriority` stores the original category (early birds/regular/wildcard).
    Player(string id, string name, int rank, string status = "regular", string time = ""): // Default status to "regular" (lowercase)
        playerId(id), 
        playerName(name), 
        ranking(rank), 
        registrationStatus(status), 
        registrationTime(time)
        {
            // For players loaded from CSV, originalPriority is the status read from CSV.
            // For replacement players, the 'status' parameter might be "Replacement",
            // but originalPriority should reflect their category based on rank.
            // The `addReplacementPlayer` function will correctly set this.
            originalPriority = status; 
        }

    friend ostream& operator<<(ostream& os, const Player& p) {
        os << "ID: " << p.playerId << ", Name: " << p.playerName << ", Rank: " << p.ranking;
        return os;
    }

    // Needed for sorting by MatchScheduler (used by custom sort logic)
    // Compares based on numerical rank (lower is better)
    bool operator<(const Player& other) const {
        return ranking < other.ranking;
    }
    // Equality based on Player ID (string)
    bool operator==(const Player& other) const {
        return playerId == other.playerId;
    }
    bool operator!=(const Player& other) const {
        return !(*this == other);
    }
};

// Custom Insertion Sort function for Player array
// Sorts by numerical rank (lower is better)
void insertionSortPlayers(Player arr[], int n) {
    for (int i = 1; i < n; ++i) {
        Player key = arr[i];
        int j = i - 1;

        while (j >= 0 && arr[j].ranking > key.ranking) { // Uses Player::ranking for comparison
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


// --- Match Struct (from original tournament system) ---
struct Match {
    static int nextMatchIdCounter; // Static counter for unique match IDs
    int matchId;
    Player player1;
    Player player2;
    Player winner; // Winner player object
    bool played;   // Flag to indicate if the match has been played

    Match(Player p1 = Player(), Player p2 = Player()) : player1(p1), player2(p2), played(false) {
        matchId = nextMatchIdCounter++;
        // 'winner' is a default Player() (ID "", Name "", Rank 0) until determined
    }

    friend ostream& operator<<(ostream& os, const Match& m) {
        os << "Match ID: " << m.matchId << " | " << m.player1.playerName << " vs " << m.player2.playerName;
        if (m.played) {
            if (m.winner.playerId != "") { // Check if winner is a valid player (empty string ID is default)
                os << " | Winner: " << m.winner.playerName;
            } else {
                os << " | Winner: Undecided/Draw"; 
            }
        } else {
            os << " | Status: Pending";
        }
        return os;
    }
};

int Match::nextMatchIdCounter = 1; // Initialize static member for Match IDs

// --- CustomQueue Class Template (from original tournament system) ---
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

    // Copy constructor (deep copy)
    CustomQueue(const CustomQueue& other) : frontNode(nullptr), rearNode(nullptr), count(0) {
        Node* current = other.frontNode;
        while (current != nullptr) {
            this->enqueue(current->data);
            current = current->next;
        }
    }

    // Assignment operator (deep copy)
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


// --- CustomStack Class Template (from original tournament system before fixes :(((  )))) ---
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

    // Copy constructor (deep copy)
    CustomStack(const CustomStack& other) : topNode(nullptr), count(0) {
        if (other.topNode == nullptr) {
            return;
        }
        // Temporary array for copying elements in correct order
        T* tempArray = new T[other.count];
        int i = 0;
        Node* current = other.topNode;
        while(current != nullptr) {
            tempArray[i++] = current->data; 
            current = current->next;
        }
        // Push elements from the end of tempArray first to maintain stack order
        for (int j = i - 1; j >= 0; --j) {
            this->push(tempArray[j]);
        }
        delete[] tempArray; 
    }

    // Assignment operator (deep copy)
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

// ---Struct from the old system
struct HistoricalMatch {
    int matchId;
    string player1Id; // Changed from int to string
    string player1Name;
    string player2Id; // Changed from int to string
    string player2Name;
    string winnerId; // Changed from int to string
    string winnerName;

    HistoricalMatch(int mId = 0, string p1Id = "", string p1N = "N/A", // Changed default id to empty string
                    string p2Id = "", string p2N = "N/A",
                    string wId = "", string wN = "N/A")
        : matchId(mId), player1Id(p1Id), player1Name(p1N),
          player2Id(p2Id), player2Name(p2N),
          winnerId(wId), winnerName(wN) {}

    friend ostream& operator<<(ostream& os, const HistoricalMatch& hm) {
        os << "Match ID: " << hm.matchId << " | P1: " << hm.player1Name << " (ID:" << hm.player1Id << ")"
           << " vs P2: " << hm.player2Name << " (ID:" << hm.player2Id << ")";
        if (hm.winnerId != "") { // Assuming empty string player ID is not a valid playing ID
            os << " | Winner: " << hm.winnerName << " (ID:" << hm.winnerId << ")";
        } else {
            os << " | Winner: TBD / Draw / No valid winner";
        }
        return os;
    }
};

// --- PlayerStats Struct
struct PlayerStats {
    string playerId; // Changed from int to string
    string playerName;
    int initialRank; 
    int wins;
    int losses;

    PlayerStats(string id = "", string name = "N/A", int r = 9999) // Changed default id to empty string
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


// --- GameResultLogger Class
class GameResultLogger {
private:
    CustomStack<HistoricalMatch> recentMatchesLog;         
    CustomQueue<HistoricalMatch> allMatchesChronologicalLog; 

    PlayerStats playerStatsArray[MAX_PLAYERS_UNIVERSAL]; 
    int numTrackedPlayers;                               

    // Helper to find a player's stats in the array (string playerId)
    PlayerStats* findPlayerStatsInArray(string playerId) { 
        for (int i = 0; i < numTrackedPlayers; ++i) {
            if (playerStatsArray[i].playerId == playerId) {
                return &playerStatsArray[i];
            }
        }
        return nullptr; // Not found
    }
    // Const version for read-only access
    const PlayerStats* findPlayerStatsInArray(string playerId) const { 
        for (int i = 0; i < numTrackedPlayers; ++i) {
            if (playerStatsArray[i].playerId == playerId) {
                return &playerStatsArray[i];
            }
        }
        return nullptr; // Not found
    }

public:
    GameResultLogger() : numTrackedPlayers(0) {} 

    // Initializes a stats entry for a new player.
    void initializePlayerForStats(const Player& player) {
        if (findPlayerStatsInArray(player.playerId) == nullptr) { 
            if (numTrackedPlayers < MAX_PLAYERS_UNIVERSAL) {
                playerStatsArray[numTrackedPlayers++] = PlayerStats(player.playerId, player.playerName, player.ranking); 
            } else {
                cerr << "Error: Cannot track stats for more players. Max capacity (" << MAX_PLAYERS_UNIVERSAL << ") reached." << endl;
            }
        } 
    }

    // Records the outcome of a completed match.
    void recordMatchOutcome(const Match& completedMatch) {
        if (!completedMatch.played) {
            return;
        }
        if (completedMatch.winner.playerId == "" || completedMatch.winner.playerName == "N/A") { 
            cerr << "Warning: Match (ID: " << completedMatch.matchId 
                 << ") outcome is unclear (invalid winner). Performance log update skipped." << endl;
            return; 
        }

        HistoricalMatch histMatch(
            completedMatch.matchId,
            completedMatch.player1.playerId, completedMatch.player1.playerName, 
            completedMatch.player2.playerId, completedMatch.player2.playerName, 
            completedMatch.winner.playerId, completedMatch.winner.playerName    
        );

        recentMatchesLog.push(histMatch);             
        allMatchesChronologicalLog.enqueue(histMatch); 

        // Update win/loss stats for both participating players using the array
        PlayerStats* p1Stats = findPlayerStatsInArray(completedMatch.player1.playerId); 
        PlayerStats* p2Stats = findPlayerStatsInArray(completedMatch.player2.playerId); 

        if (p1Stats != nullptr) {
            if (completedMatch.winner.playerId == completedMatch.player1.playerId) { 
                p1Stats->recordWin();
            } else {
                p1Stats->recordLoss();
            }
        } else {
            cerr << "Warning: Player " << completedMatch.player1.playerName << " (ID: " << completedMatch.player1.playerId 
                 << ") not found in stats tracking array. Performance not updated." << endl;
        }

        if (p2Stats != nullptr) {
            if (completedMatch.winner.playerId == completedMatch.player2.playerId) { 
                p2Stats->recordWin();
            } else {
                p2Stats->recordLoss();
            }
        } else {
             cerr << "Warning: Player " << completedMatch.player2.playerName << " (ID: " << completedMatch.player2.playerId 
                 << ") not found in stats tracking array. Performance not updated." << endl;
        }
    }

    void displayRecentMatches(int numToDisplay = 5) const {
        int actualDisplayCount = numToDisplay;
        if (recentMatchesLog.size() < actualDisplayCount) {
            actualDisplayCount = recentMatchesLog.size();
        }
        cout << "\n--- Recent Match Results (Last " << actualDisplayCount << " / " << recentMatchesLog.size() << " Total Recent) ---" << endl;
        
        if (recentMatchesLog.isEmpty()) {
            cout << "No match results have been recorded yet." << endl;
        } else {
            CustomStack<HistoricalMatch> tempStack = recentMatchesLog; 
            int displayedCount = 0;
            while (!tempStack.isEmpty() && displayedCount < numToDisplay) {
                cout << tempStack.pop() << endl; 
                displayedCount++;
            }
        }
        cout << "--------------------------------------------------------------------" << endl;
    }

    void displayAllRecordedMatches() const {
        cout << "\n--- All Recorded Matches (" << allMatchesChronologicalLog.size() << " Total) ---" << endl;
        if (allMatchesChronologicalLog.isEmpty()) {
            cout << "No matches have been recorded in the chronological log." << endl;
        } else {
            CustomQueue<HistoricalMatch> tempQueue = allMatchesChronologicalLog; 
            int counter = 1;
            while (!tempQueue.isEmpty()) {
                cout << counter++ << ". " << tempQueue.dequeue() << endl;
            }
        }
        cout << "--------------------------------------------------------------------" << endl;
    }

    void displaySinglePlayerPerformance(string playerId) const { 
        cout << "\n--- Full Performance Record for Player ID: " << playerId << " ---" << endl;
        const PlayerStats* playerStats = findPlayerStatsInArray(playerId); 
        if (playerStats != nullptr) { 
            cout << *playerStats << endl; 
            
            // List all matches involving this player from the chronological log
            cout << "  Matches involving " << playerStats->playerName << ":" << endl;
            CustomQueue<HistoricalMatch> tempAllMatches = allMatchesChronologicalLog; 
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
        cout << "\n--- All Player Performance Summaries (" << numTrackedPlayers << " Players Tracked) ---" << endl;
        if (numTrackedPlayers == 0) { // Typo fix: numTrackers -> numTrackedPlayers
            cout << "No player performance data available." << endl;
        } else {
            for (int i = 0; i < numTrackedPlayers; ++i) {
                cout << playerStatsArray[i] << endl;
            }
        }
        cout << "--------------------------------------------------------------------" << endl;
    }
};


// --- MatchScheduler Class (from original tournament system) ---
class MatchScheduler {
private:
    Player initialPlayers[MAX_PLAYERS_UNIVERSAL]; // Array to store all initially added players
    int numInitialPlayers;                        // Count of players in initialPlayers

    CustomQueue<Player> waitingPlayersQueue; 
    CustomQueue<Match> scheduledMatchesQueue; 
    CustomQueue<Player> winnersQueue;         
    
    GameResultLogger& resultLogger; 

    // Helper to sort players by rank (lower rank is better)
    void sortPlayersByRank(Player arr[], int n) {
        insertionSortPlayers(arr, n); 
    }

    // Helper struct for group stage results, for sorting
    struct GroupPlayerResult {
        Player player;
        int groupWins;
    };

    // Custom Insertion Sort for GroupPlayerResult (sort by wins desc, then rank asc)
    void insertionSortGroupResults(GroupPlayerResult arr[], int n) {
        for (int i = 1; i < n; ++i) {
            GroupPlayerResult key = arr[i];
            int j = i - 1;

            while (j >= 0 && (arr[j].groupWins < key.groupWins || (arr[j].groupWins == key.groupWins && arr[j].player.ranking > key.player.ranking))) {
                arr[j + 1] = arr[j];
                j = j - 1;
            }
            arr[j + 1] = key;
        }
    }

public:
    MatchScheduler(GameResultLogger& logger) : numInitialPlayers(0), resultLogger(logger) {}

    // Adds a player to the tournament and initializes them for stats tracking.
    void addPlayer(const Player& player) {
        if (numInitialPlayers < MAX_PLAYERS_UNIVERSAL) {
            // Prevent adding player with duplicate ID (if IDs are read from CSV)
            for(int i=0; i < numInitialPlayers; ++i) {
                if(initialPlayers[i].playerId == player.playerId) { // Changed player.id to player.playerId
                    cerr << "Error: Player with ID " << player.playerId << " (" << initialPlayers[i].playerName 
                         << ") already exists. Cannot add " << player.playerName << " with the same ID." << endl;
                    return; // Skip adding this player
                }
            }
            initialPlayers[numInitialPlayers++] = player;
            resultLogger.initializePlayerForStats(player); 
        } else {
            cerr << "Error: Cannot add more players. Maximum capacity (" << MAX_PLAYERS_UNIVERSAL << ") reached." << endl;
        }
    }
    
    int getNumInitialPlayers() const { 
        return numInitialPlayers;
    }

    // Initializes the tournament by sorting all initial players.
    void initializeTournament() {
        if (numInitialPlayers == 0) {
            cout << "No players added to the tournament to initialize." << endl;
            return;
        }

        // Sort all registered players by rank
        sortPlayersByRank(initialPlayers, numInitialPlayers); 
        cout << "\n--- All " << numInitialPlayers << " Players Sorted by Rank (Initial Seeding) ---" << endl;
        for (int i = 0; i < numInitialPlayers; ++i) {
            cout << (i + 1) << ". " << initialPlayers[i].playerName << " (Rank: " << initialPlayers[i].ranking << ", Type: " << initialPlayers[i].originalPriority << ")" << endl;
        }
        cout << "------------------------------------" << endl;
    }

    // Group Stage Method
    void runGroupStage(int numGroups) {
        if (numGroups <= 0) {
            cerr << "Error: Number of groups must be positive for group stage." << endl;
            return;
        }
        
        // Expected players for full group stage: 6 * 4 + 10 * 4 = 24 + 40 = 64
        if (numInitialPlayers < (6 * 4 + 10 * 4)) { 
            cerr << "Warning: Not enough players (" << numInitialPlayers 
                 << ") to form " << numGroups << " groups with required compositions. Skipping group stage or adjusting." << endl;
            // If there's only one player after initialization, they are the champion.
            if (numInitialPlayers == 1) {
                winnersQueue.enqueue(initialPlayers[0]);
                cout << initialPlayers[0].playerName << " is the sole player and advances directly as the champion after group stage." << endl;
            } else if (numInitialPlayers > 0) {
                // If there are players but not enough for full groups,
                // just advance all available players directly to the main bracket.
                cout << "Not enough players for proper group stage. Advancing all " << numInitialPlayers << " players directly to main bracket." << endl;
                for (int i = 0; i < numInitialPlayers; ++i) {
                    winnersQueue.enqueue(initialPlayers[i]);
                }
            }
            return;
        }
        
        cout << "\n--- Group Division: Starting Group Stage with " << numGroups << " Groups ---" << endl;

        // Categorize players into temporary queues based on registration type
        // Players are drawn from initialPlayers, which is already sorted by rank (strongest first)
        CustomQueue<Player> earlyBirdsQ;
        CustomQueue<Player> regularsQ;
        CustomQueue<Player> wildcardsQ;

        for (int i = 0; i < numInitialPlayers; ++i) {
            if (initialPlayers[i].originalPriority == "early birds") { // Use exact string from CSV
                earlyBirdsQ.enqueue(initialPlayers[i]);
            } else if (initialPlayers[i].originalPriority == "regular") { // Use exact string from CSV
                regularsQ.enqueue(initialPlayers[i]);
            } else if (initialPlayers[i].originalPriority == "wildcard") { // Use exact string from CSV
                wildcardsQ.enqueue(initialPlayers[i]);
            } else {
                cerr << "Warning: Unknown original priority for player: " << initialPlayers[i].playerName << " ('" << initialPlayers[i].originalPriority << "'). Skipping for group stage." << endl;
            }
        }

        // Array of queues for groups (max 16 groups)
        CustomQueue<Player> groupQueues[16]; 
        int currentGroupIndex = 0;

        // Form 6 groups of (1 early bird, 2 regular, 1 wildcard)
        for (int i = 0; i < 6; ++i) {
            if (earlyBirdsQ.isEmpty() || regularsQ.size() < 2 || wildcardsQ.isEmpty()) {
                cerr << "Error: Not enough players for specific group composition (1 Early Bird, 2 Regular, 1 Wildcard) for group " << (i + 1) << ". Cannot complete group stage." << endl;
                // Enqueue remaining players to winnersQueue if group stage cannot complete
                while(!earlyBirdsQ.isEmpty()) winnersQueue.enqueue(earlyBirdsQ.dequeue());
                while(!regularsQ.isEmpty()) winnersQueue.enqueue(regularsQ.dequeue());
                while(!wildcardsQ.isEmpty()) winnersQueue.enqueue(wildcardsQ.dequeue());
                return; 
            }
            groupQueues[currentGroupIndex].enqueue(earlyBirdsQ.dequeue());
            groupQueues[currentGroupIndex].enqueue(regularsQ.dequeue());
            groupQueues[currentGroupIndex].enqueue(regularsQ.dequeue());
            groupQueues[currentGroupIndex].enqueue(wildcardsQ.dequeue());
            cout << "  Group " << (currentGroupIndex + 1) << " formed (1 Early Bird, 2 Regular, 1 Wildcard)." << endl;
            currentGroupIndex++;
        }

        // Form 10 groups of (1 early bird, 3 regular)
        for (int i = 0; i < 10; ++i) {
            if (earlyBirdsQ.isEmpty() || regularsQ.size() < 3) {
                cerr << "Error: Not enough players for specific group composition (1 Early Bird, 3 Regular) for group " << (i + 1) << ". Cannot complete group stage." << endl;
                // Enqueue remaining players to winnersQueue if group stage cannot complete
                while(!earlyBirdsQ.isEmpty()) winnersQueue.enqueue(earlyBirdsQ.dequeue());
                while(!regularsQ.isEmpty()) winnersQueue.enqueue(regularsQ.dequeue());
                while(!wildcardsQ.isEmpty()) winnersQueue.enqueue(wildcardsQ.dequeue());
                return;
            }
            groupQueues[currentGroupIndex].enqueue(earlyBirdsQ.dequeue());
            groupQueues[currentGroupIndex].enqueue(regularsQ.dequeue());
            groupQueues[currentGroupIndex].enqueue(regularsQ.dequeue());
            groupQueues[currentGroupIndex].enqueue(regularsQ.dequeue());
            cout << "  Group " << (currentGroupIndex + 1) << " formed (1 Early Bird, 3 Regular)." << endl;
            currentGroupIndex++;
        }

        // Process each group: play matches and determine top 2
        int playersAdvancedFromGroupStage = 0;
        for (int i = 0; i < currentGroupIndex; ++i) { // Iterate through the 16 formed groups
            cout << "\n--- Processing Group " << (i + 1) << " ---" << endl;
            
            if (groupQueues[i].size() != 4) { 
                cerr << "Fatal Error: Group " << (i + 1) << " does not have exactly 4 players (" << groupQueues[i].size() << "). This should not happen if player counts are correct. Cannot run matches for this group." << endl;
                while (!groupQueues[i].isEmpty()) groupQueues[i].dequeue(); // Clear the group
                continue; 
            }

            Player groupPlayersArray[4];
            for (int k = 0; k < 4; ++k) {
                groupPlayersArray[k] = groupQueues[i].dequeue();
            }

            // Initialize local group win tracking
            GroupPlayerResult groupResults[4];
            for (int k = 0; k < 4; ++k) {
                groupResults[k] = {groupPlayersArray[k], 0};
            }

            // Simulate matches for 4 games per player within the group
            // Total matches per group = 4 players * 4 games / 2 players per match = 8 matches
            int matchPairings[8][2] = {
                {0,1}, {2,3}, // First set of matches
                {0,2}, {1,3}, // Second set
                {0,3}, {1,2}, // Third set
                {0,1}, {2,3}  // Rematches for 4th game per player (P0,P1 play again; P2,P3 play again)
            };

            for (int p = 0; p < 8; ++p) {
                Player p1 = groupPlayersArray[matchPairings[p][0]];
                Player p2 = groupPlayersArray[matchPairings[p][1]];
                Match currentMatch(p1, p2);
                
                // Simulate match outcome: RANDOMLY (50/50 chance for either player)
                if (rand() % 2 == 0) { 
                    currentMatch.winner = currentMatch.player1;
                } else { 
                    currentMatch.winner = currentMatch.player2;
                }
                currentMatch.played = true; 
                resultLogger.recordMatchOutcome(currentMatch); // Record for logging

                // Update group stage wins
                for(int k=0; k<4; ++k) {
                    if(groupResults[k].player.playerId == currentMatch.winner.playerId) { 
                        groupResults[k].groupWins++;
                        break; 
                    }
                }
                cout << "  Match: " << currentMatch.player1.playerName << " vs " << currentMatch.player2.playerName
                     << " -> Winner: " << currentMatch.winner.playerName << endl;
            }

            // Determine Top 2 from the group
            insertionSortGroupResults(groupResults, 4); // Sort by group wins (desc), then rank (asc)

            cout << "  Group " << (i + 1) << " Standings (Top 2 advance):" << endl;
            for(int k=0; k<4; ++k) {
                cout << "    " << (k+1) << ". " << groupResults[k].player.playerName << " (Group Wins: " << groupResults[k].groupWins << ")" << endl;
            }

            // Enqueue top 2 players to winnersQueue
            winnersQueue.enqueue(groupResults[0].player);
            playersAdvancedFromGroupStage++;
            winnersQueue.enqueue(groupResults[1].player);
            playersAdvancedFromGroupStage++;
            cout << "  " << groupResults[0].player.playerName << " and " << groupResults[1].player.playerName << " advance from Group " << (i+1) << "." << endl;
        }
        cout << "\n--- Group Stage Complete. " << playersAdvancedFromGroupStage << " players advanced to main bracket. ---" << endl;
    }


    // Creates match pairings for the next round from players in waitingPlayersQueue.
    bool createNextRoundPairings() {
        if (waitingPlayersQueue.size() < 2) {
            return false; 
        }

        cout << "\n--- Creating Pairings for Next Round ---" << endl;

        // Extract players from waiting queue to a temporary array for pairing logic
        Player roundPlayersArray[MAX_PLAYERS_UNIVERSAL];
        int numRoundPlayers = 0;
        while(!waitingPlayersQueue.isEmpty() && numRoundPlayers < MAX_PLAYERS_UNIVERSAL) {
            roundPlayersArray[numRoundPlayers++] = waitingPlayersQueue.dequeue();
        }
        
        // Ensure players are sorted by rank before pairing for next round
        sortPlayersByRank(roundPlayersArray, numRoundPlayers);

        int i = 0, j = numRoundPlayers - 1;
        while (i < j) {
            Match newMatch(roundPlayersArray[i], roundPlayersArray[j]);
            scheduledMatchesQueue.enqueue(newMatch);
            cout << "Scheduled: " << newMatch.player1.playerName << " (Rank: " << newMatch.player1.ranking << ") vs " << newMatch.player2.playerName << " (Rank: " << newMatch.player2.ranking << ")" << endl;
            i++;
            j--;
        }

        if (i == j) { // One player remains in the middle - receives a bye
            Player byePlayer = roundPlayersArray[i];
            cout << byePlayer.playerName << " gets a BYE and advances directly to the winners' pool." << endl;
            winnersQueue.enqueue(byePlayer); 
        }
        cout << "------------------------------------" << endl;
        return !scheduledMatchesQueue.isEmpty() || (numRoundPlayers > 0 && i == j);
    }

    // Simulates playing scheduled matches and processes their outcomes.
    void playAndProcessMatches() {
        if (scheduledMatchesQueue.isEmpty()) {
            return; 
        }
        
        cout << "\n--- Playing and Processing Matches ---" << endl;
        while (!scheduledMatchesQueue.isEmpty()) {
            Match currentMatch = scheduledMatchesQueue.dequeue();

            if (rand() % 2 == 0) { 
                currentMatch.winner = currentMatch.player1;
            } else { 
                currentMatch.winner = currentMatch.player2;
            }
            currentMatch.played = true;

            cout << currentMatch.player1.playerName << " vs " << currentMatch.player2.playerName
                 << " -> Winner: " << currentMatch.winner.playerName << endl;
            
            resultLogger.recordMatchOutcome(currentMatch); 

            winnersQueue.enqueue(currentMatch.winner); 
        }
        cout << "------------------------------------" << endl;
    }

    // Advances winners from the winnersQueue to the waitingPlayersQueue for the next round.
    bool advanceToNextRound() {
        if (winnersQueue.isEmpty() && waitingPlayersQueue.isEmpty()) {
            return false;
        }

        if (winnersQueue.size() == 1 && waitingPlayersQueue.isEmpty() && scheduledMatchesQueue.isEmpty()) {
            Player finalWinnerCandidate = winnersQueue.peek(); 
            waitingPlayersQueue.enqueue(winnersQueue.dequeue()); 
            cout << "\n--- Advancing Final Player ---" << endl;
            cout << finalWinnerCandidate.playerName << " is the sole remaining player." << endl;
            cout << "------------------------------------" << endl;
            return false; 
        }
        
        cout << "\n--- Advancing Winners to Next Round's Waiting Pool ---" << endl;
        while (!winnersQueue.isEmpty()) {
            Player winnerToAdvance = winnersQueue.dequeue();
            waitingPlayersQueue.enqueue(winnerToAdvance);
            cout << winnerToAdvance.playerName << " advances." << endl;
        }
        cout << "------------------------------------" << endl;

        return waitingPlayersQueue.size() >= 1; 
    }

    // Display functions for queues (mostly for debugging)
    void displayScheduledMatches() const { 
        cout << "\n--- Current Scheduled Matches (" << scheduledMatchesQueue.size() << ") ---" << endl;
        if (scheduledMatchesQueue.isEmpty()) {
            cout << "No matches currently scheduled." << endl;
        } else {
            CustomQueue<Match> tempQueue = scheduledMatchesQueue; 
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
            CustomQueue<Player> tempQueue = waitingPlayersQueue; 
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
            CustomQueue<Player> tempQueue = winnersQueue; 
            while (!tempQueue.isEmpty()) {
                cout << tempQueue.dequeue() << endl;
            }
        }
        cout << "------------------------------------" << endl;
    }

    // Checks if the tournament has concluded
    bool isTournamentOver() const {
        return (waitingPlayersQueue.size() == 1 && scheduledMatchesQueue.isEmpty() && winnersQueue.isEmpty());
    }

    // Returns the tournament champion if the tournament is over.
    Player getTournamentWinner() const {
        if (isTournamentOver()) {
            return waitingPlayersQueue.peek(); 
        }
        if (winnersQueue.size() == 1 && waitingPlayersQueue.isEmpty() && scheduledMatchesQueue.isEmpty()) {
            return winnersQueue.peek();
        }
        return Player(); // Returns a default Player (ID "", Name "N/A") if no winner yet or error state
    }
    Player getInitialPlayer(int index) const { // For one-player fallback
        if (index >= 0 && index < numInitialPlayers) {
            return initialPlayers[index];
        }
        return Player(); 
    }
};

// --- PlayerQueue from provided code (modified to use new Player struct) ---
struct QueueNode {
    Player playerData;
    QueueNode* nextNode;

    QueueNode(Player player) : 
        playerData(player), 
        nextNode(nullptr) {}
};

// Custom queue using linked list
class PlayerQueue {
private:
    QueueNode* frontNode;
    QueueNode* backNode;
    int queueSize;

public:
    // Adding public getter to access the front node
    QueueNode* getFrontNode() {
        return frontNode;
    }

    QueueNode* getBackNode() {
        return backNode;
    }

    // Constructor
    PlayerQueue() : 
        frontNode(nullptr), 
        backNode(nullptr), 
        queueSize(0) {}

    // Destructor for cleaning up memory
    ~PlayerQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    // Check if queue is empty
    bool isEmpty() {
        return frontNode == nullptr;
    }

    // Get queue size
    int size() {
        return queueSize;
    }

    // Add player to the back of queue
    void enqueue(Player player) {
        QueueNode* newNode = new QueueNode(player);

        if (isEmpty()) {
            frontNode = backNode = newNode;
        }
        else {
            backNode->nextNode = newNode;
            backNode = newNode;
        }
        queueSize++;
    }

    // Remove and return player from the front of queue
    Player dequeue() {
        if (isEmpty()) {
            return Player(); // Return empty player
        }

        QueueNode* removedNode = frontNode;
        Player removedPlayer = frontNode->playerData;

        frontNode = frontNode->nextNode;
        if (frontNode == nullptr) {
            backNode = nullptr;
        }

        delete removedNode;
        queueSize--;

        return removedPlayer;
    }

    // Peek at front player without removing
    Player front() {
        if (isEmpty()) {
            cout << "Registration queue is empty." << endl;
            return Player();
        }
        return frontNode->playerData;
    }

    // HELPER: Create temporarily sorted linked list for display
    QueueNode* createSortedDisplayList() 
    {
        if (isEmpty()) return nullptr;

        QueueNode* sortedHead = nullptr;
        QueueNode* current = frontNode;

        // Copy all nodes to a new sorted list
        while (current != nullptr) {
            // Create a copy of the player data to avoid modifying original
            Player playerCopy = current->playerData; 
            QueueNode* newNode = new QueueNode(playerCopy);

            // Insert in sorted order (based on ranking)
            if (sortedHead == nullptr || newNode->playerData.ranking < sortedHead->playerData.ranking) 
            {
                newNode->nextNode = sortedHead;
                sortedHead = newNode;
            }
            else {
                QueueNode* temp = sortedHead;
                while (temp->nextNode != nullptr && temp->nextNode->playerData.ranking < newNode->playerData.ranking) 
                {
                    temp = temp->nextNode;
                }
                newNode->nextNode = temp->nextNode;
                temp->nextNode = newNode;
            }
            current = current->nextNode;
        }

        return sortedHead;
    }

    // HELPER: Delete sorted list
    void deleteSortedList(QueueNode* head) {
        while (head != nullptr) {
            QueueNode* temp = head;
            head = head->nextNode;
            delete temp;
        }
    }


    // Display all players in queue
    void displayQueue() {
        if (isEmpty()) {
            cout << "No players found in this queue." << endl;
            return;
        }

        QueueNode* sortedHead = createSortedDisplayList();

        cout << "Position | Player ID | Player Name          | Ranking | Status" << endl;
        cout << "---------|-----------|----------------------|---------|----------------" << endl;

        QueueNode* currentNode = sortedHead;
        int position = 1;

        while (currentNode != nullptr) {
            cout << position << "        | " << currentNode->playerData.playerId
                << "    | " << currentNode->playerData.playerName;

            // Pad name to 20 characters for alignment
            for (int i = currentNode->playerData.playerName.length(); i < 20; i++) {
                cout << " ";
            }

            cout << "| " << currentNode->playerData.ranking
                << "      | " << currentNode->playerData.registrationStatus << endl;

            currentNode = currentNode->nextNode;
            position++;
        }
        cout << "Total players in queue: " << queueSize << "\n" << endl;
        deleteSortedList(sortedHead); // Clean up the temporary sorted list
    }

    // Find and remove specific player (when withdrawing a player)
    Player removePlayer(string playerId) 
    {  
        Player removedPlayer;  // Stores the removed player data

        if (isEmpty()) {
            return Player(); // Returns empty player if the queue is empty
        }

        // When the player is at the front
        if (frontNode->playerData.playerId == playerId) {
            removedPlayer = dequeue();  // Stores the player data
            return removedPlayer;  // Returns player data
        }

        // Search for player in the middle or back of the node
        QueueNode* currentNode = frontNode;
        while (currentNode->nextNode != nullptr) {
            if (currentNode->nextNode->playerData.playerId == playerId) {
                QueueNode* removedNode = currentNode->nextNode;
                removedPlayer = removedNode->playerData;  // Stores player data
                currentNode->nextNode = removedNode->nextNode;

                // Update back pointer when removing the last node
                if (removedNode == backNode) {
                    backNode = currentNode;
                }

                delete removedNode;
                queueSize--;
                return removedPlayer;  // Returns player data
            }
            currentNode = currentNode->nextNode;
        }

        return Player(); // Returns empty player if not found
    }

    // Update player status (when checking in a player)
    bool updatePlayerStatus(string playerId, string newStatus) {
        QueueNode* currentNode = frontNode;

        while (currentNode != nullptr) 
        {
            if (currentNode->playerData.playerId == playerId) 
            {
                string oldStatus = currentNode->playerData.registrationStatus;
                currentNode->playerData.registrationStatus = newStatus;
                cout << "Player " << currentNode->playerData.playerName
                    << " status has been updated from '" << oldStatus << "' to '" << newStatus << "'" << endl;
                return true;
            }
            currentNode = currentNode->nextNode;
        }

        cout << "Player " << playerId << " not found for status update." << endl;
        return false;
    }

    void insertByRanking(Player player) 
    {
        QueueNode* newNode = new QueueNode(player);

        // If queue is empty or new player has higher ranking (The lower the number, the higher the ranking)
        if (isEmpty() || player.ranking < frontNode->playerData.ranking) 
        {
            newNode->nextNode = frontNode;
            frontNode = newNode;
            if (backNode == nullptr) 
            {
                backNode = newNode;
            }
        }
        else {
            // Find the correct position based on the ranking
            QueueNode* currentNode = frontNode;
            while (currentNode->nextNode != nullptr &&
                currentNode->nextNode->playerData.ranking < player.ranking) 
            {
                currentNode = currentNode->nextNode;
            }

            newNode->nextNode = currentNode->nextNode;
            currentNode->nextNode = newNode;

            // Update back pointer if inserting at the end
            if (newNode->nextNode == nullptr) 
            {
                backNode = newNode;
            }
        }
        queueSize++;
    }
};

// --- PriorityPlayerQueue from provided code (modified to use new Player struct) ---
class PriorityPlayerQueue {
private:
    QueueNode* frontNode;
    int queueSize;

public:
    PriorityPlayerQueue() : 
        frontNode(nullptr), 
        queueSize(0) {}

    ~PriorityPlayerQueue() 
    {
        while (!isEmpty()) 
        {
            dequeue();
        }
    }

    bool isEmpty() 
    {
        return frontNode == nullptr;
    }

    int size() 
    {
        return queueSize;
    }

    // Insert player based on priority type (early birds, regular, wildcard)
    void enqueue(Player player) 
    {
        QueueNode* newNode = new QueueNode(player);

        // If queue is empty or new player has highest priority
        // Using originalPriority for determining priority order
        if (isEmpty() || getPriority(player.originalPriority) > getPriority(frontNode->playerData.originalPriority)) {
            newNode->nextNode = frontNode;
            frontNode = newNode;
        }
        else {
            // Find the correct position based on priority
            QueueNode* currentNode = frontNode;
            while (currentNode->nextNode != nullptr &&
                getPriority(currentNode->nextNode->playerData.originalPriority) >= getPriority(player.originalPriority)) {
                currentNode = currentNode->nextNode;
            }

            newNode->nextNode = currentNode->nextNode;
            currentNode->nextNode = newNode;
        }

        queueSize++;
    }

    Player dequeue() 
    {
        if (isEmpty()) {
            return Player();
        }

        QueueNode* removedNode = frontNode;
        Player removedPlayer = frontNode->playerData;

        frontNode = frontNode->nextNode;
        delete removedNode;
        queueSize--;

        return removedPlayer;
    }

    // Get priority value for comparison (matching CSV strings)
    int getPriority(string status) 
    {
        if (status == "early birds") return 3;
        if (status == "regular") return 2;
        if (status == "wildcard") return 1;
        return 0; // Default or unknown status (lowest priority)
    }

    void displayQueue() 
    {
        if (isEmpty()) 
        {
            cout << "Priority queue is empty." << endl;
            return;
        }

        cout << "\n--- Priority Registration Queue ---" << endl;
        cout << "Priority | Player ID | Player Name          | Status" << endl;
        cout << "---------|-----------|----------------------|----------------" << endl;

        QueueNode* currentNode = frontNode;
        int position = 1;

        while (currentNode != nullptr) {
            cout << position << "        | " << currentNode->playerData.playerId
                << "    | " << currentNode->playerData.playerName;

            for (int i = currentNode->playerData.playerName.length(); i < 20; i++) {
                cout << " ";
            }

            cout << "| " << currentNode->playerData.originalPriority << endl; // Display original priority
            currentNode = currentNode->nextNode;
            position++;
        }
        cout << "Total players in priority queue: " << queueSize << "\n" << endl;
    }
};

// --- TournamentRegistrationSystem from Guled
class TournamentRegistrationSystem {
private:
    PlayerQueue mainRegistrationQueue;
    PriorityPlayerQueue priorityQueue;
    PlayerQueue checkedInQueue;
    PlayerQueue withdrawnPlayersQueue;
    PlayerQueue replacementQueue; // This queue just holds the replacement players

    // These counters track players *currently active* in main or checked-in queues
    int earlyBirdCount;
    int regularCount;
    int wildcardCount;
    int replacementCount; // Total replacements added
    int originalPriorityCount; // Total players loaded from file initially

    // HELPER: Determine original priority based on rank, used for new players (like replacements)
    // Needs to return the exact string matching CSV
    string determineOriginalPriorityByRank(int rank) {
        if (rank >= 1 && rank <= 16) return "early birds";
        else if (rank >= 59 && rank <= 64) return "wildcard";
        else return "regular";
    }

    // Recalculates current counts based on players in active queues
    void recalculateCounters() {
        earlyBirdCount = 0;
        regularCount = 0;
        wildcardCount = 0;

        // Iterate through main registration queue
        QueueNode* current = mainRegistrationQueue.getFrontNode();
        while (current != nullptr) {
            if (current->playerData.originalPriority == "early birds") earlyBirdCount++;
            else if (current->playerData.originalPriority == "regular") regularCount++;
            else if (current->playerData.originalPriority == "wildcard") wildcardCount++;
            current = current->nextNode;
        }

        // Iterate through checked-in queue
        current = checkedInQueue.getFrontNode();
        while (current != nullptr) {
            if (current->playerData.originalPriority == "early birds") earlyBirdCount++;
            else if (current->playerData.originalPriority == "regular") regularCount++;
            else if (current->playerData.originalPriority == "wildcard") wildcardCount++;
            current = current->nextNode;
        }
    }


public:
    TournamentRegistrationSystem() : 
        earlyBirdCount(0), regularCount(0), wildcardCount(0), 
        replacementCount(0), originalPriorityCount(0) {}

    // NEW GETTER: To allow main function to access checkedInQueue for tournament setup
    PlayerQueue* getCheckedInPlayersQueue() {
        return &checkedInQueue;
    }

    // Load players from file and add to appropriate queues (Modified)
    void loadPlayersFromFile(string filename) 
    {
        ifstream file(filename);
        string line;

        if (!file.is_open()) 
        {
            cout << "Error: Unable to open file: " << filename << endl;
            return;
        }

        // Skip header line
        getline(file, line); // "player_id,player_name,ranking,registration type"

        while (getline(file, line) && originalPriorityCount < MAX_PLAYERS_UNIVERSAL) // Limit players loaded
        {
            stringstream ss(line);
            string playerIdStr, playerNameStr, rankingStr, regTypeStr;

            // Read all 4 fields
            if (getline(ss, playerIdStr, ',') && 
                getline(ss, playerNameStr, ',') && 
                getline(ss, rankingStr, ',') && 
                getline(ss, regTypeStr))
            {
                try {
                    // Remove leading/trailing whitespace from regTypeStr
                    regTypeStr.erase(0, regTypeStr.find_first_not_of(" \t\r\n"));
                    regTypeStr.erase(regTypeStr.find_last_not_of(" \t\r\n") + 1);

                    int ranking = stoi(rankingStr);

                    // Create player directly with CSV status
                    Player newPlayer(playerIdStr, playerNameStr, ranking, regTypeStr); // registrationStatus is now regTypeStr
                                                                                        // originalPriority will also be regTypeStr by constructor
                    
                    priorityQueue.enqueue(newPlayer);
                    originalPriorityCount++; // Count players successfully loaded
                } catch (const invalid_argument& e) {
                    cerr << "Warning: Invalid number format in line: '" << line << "' - " << e.what() << ". Skipping." << endl;
                } catch (const out_of_range& e) {
                    cerr << "Warning: Number out of range in line: '" << line << "' - " << e.what() << ". Skipping." << endl;
                }
            } else {
                if (!line.empty()) { // Avoid warning for empty lines at EOF
                    cerr << "Warning: Malformed line (expected 4 fields): '" << line << "'. Skipping." << endl;
                }
            }
        }

        file.close();

        cout << "Players loaded successfully from " << filename << ". Total: " << originalPriorityCount << endl;
    }

    // Process priority queue into main registration queue
    void processPriorityRegistrations() 
    {
        cout << "\n--- Processing Priority Registrations ---" << endl;

        // Reset counts before processing players into main queue
        earlyBirdCount = 0;
        regularCount = 0;
        wildcardCount = 0;

        while (!priorityQueue.isEmpty()) 
        {
            Player player = priorityQueue.dequeue();
            // Count based on originalPriority (which is now from CSV)
            if (player.originalPriority == "early birds") earlyBirdCount++;
            else if (player.originalPriority == "regular") regularCount++;
            else if (player.originalPriority == "wildcard") wildcardCount++;
            else cerr << "Warning: Player " << player.playerName << " has unknown original priority: '" << player.originalPriority << "'." << endl;

            mainRegistrationQueue.enqueue(player);
        }

        cout << "Processed players into main queue:\n"
            << "Early birds: " << earlyBirdCount << "\n" // Adjusted output string
            << "Regular: " << regularCount << "\n"
            << "Wildcard: " << wildcardCount << "\n" << endl;
    }

    // Handle player check-in
    void checkInPlayer() 
    {
        if (mainRegistrationQueue.isEmpty()) 
        {
            cout << "No players in registration queue to check in." << endl;
            return;
        }

        Player player = mainRegistrationQueue.dequeue();

        player.registrationStatus = "Checked In"; // Update status
        checkedInQueue.enqueue(player);

        cout << "Player " << player.playerName << " (" << player.playerId
            << ") has been checked in successfully. [Original Priority: "
            << player.originalPriority << "]" << endl; // Use player.originalPriority
    }

    // Handle player withdrawal
    void withdrawPlayer(string playerId) 
    {
        Player withdrawnPlayer;

        // Try to remove from main registration queue first
        withdrawnPlayer = mainRegistrationQueue.removePlayer(playerId);

        bool found = (withdrawnPlayer.playerId != "");

        if (!found) { 
            // If not found, try in checked-in queue
            withdrawnPlayer = checkedInQueue.removePlayer(playerId);
            found = (withdrawnPlayer.playerId != "");
        }
        
        // If found and removed from either queue
        if (found) {  
            withdrawnPlayer.registrationStatus = "Withdrawn";  // Update status to "Withdrawn"
            withdrawnPlayersQueue.enqueue(withdrawnPlayer);    // Enqueue to withdrawn players queue
            cout << "Player " << withdrawnPlayer.playerName << " (" << withdrawnPlayer.playerId << ") has been withdrawn." << endl;
        } else {
            cout << "Player " << playerId << " not found in any active queue for withdrawal." << endl;
        }
    }

    // Add replacement player (Fixed logic)
    void addReplacementPlayer(Player replacementPlayer) 
    {
        // First, determine the original priority based on the replacement's rank.
        string determinedOriginalPriority = determineOriginalPriorityByRank(replacementPlayer.ranking);
        
        // Apply rank adjustment to existing players (as per original provided code logic)
        adjustRanking(replacementPlayer.ranking); 

        replacementPlayer.originalPriority = determinedOriginalPriority; // Set original priority based on rank
        replacementPlayer.registrationStatus = "Replacement";           // Set current status to Replacement

        mainRegistrationQueue.insertByRanking(replacementPlayer); // Insert into main queue by rank
        replacementQueue.enqueue(replacementPlayer); // Add to replacement log queue
        replacementCount++; // Increment count of replacement players added

        cout << "Replacement player " << replacementPlayer.playerName
            << " (ID: " << replacementPlayer.playerId << ", Rank: " << replacementPlayer.ranking 
            << ", Original Priority: " << replacementPlayer.originalPriority << ") added to registration queue." << endl;
    }

    // Adjusts rankings of all players in active queues
    void adjustRanking(int newPlayerRank) 
    {
        // Adjust ranks in main registration queue
        QueueNode* mainQueueNode = mainRegistrationQueue.getFrontNode();
        while (mainQueueNode != nullptr) {
            if (mainQueueNode->playerData.ranking >= newPlayerRank) {
                mainQueueNode->playerData.ranking++;
            }
            mainQueueNode = mainQueueNode->nextNode;
        }

        // Adjust ranks in checked-in queue
        QueueNode* checkedInQueueNode = checkedInQueue.getFrontNode();
        while (checkedInQueueNode != nullptr) {
            if (checkedInQueueNode->playerData.ranking >= newPlayerRank) {
                checkedInQueueNode->playerData.ranking++;
            }
            checkedInQueueNode = checkedInQueueNode->nextNode;
        }
    }

    // Display all queue statuses
    void displayAllQueues() 
    {
        recalculateCounters(); // Recalculate counts every time displayed

        cout << "\n" << string(60, '-') << endl;
        cout << "TOURNAMENT REGISTRATION SYSTEM STATUS" << endl;
        cout << string(60, '-') << endl;

        cout << "\n--- Main Registration Queue ---" << endl;
        mainRegistrationQueue.displayQueue();

        cout << "\n--- Checked-In Players ---" << endl;
        checkedInQueue.displayQueue();

        cout << "\n--- Withdrawn Players ---" << endl;
        withdrawnPlayersQueue.displayQueue();

        cout << "\n--- Replacement Players (Log) ---" << endl;
        replacementQueue.displayQueue();

        cout << "\n--- Priority Summary (Active Players) ---" << endl;
        cout << "Total Active Players (Main + Checked-In): " << mainRegistrationQueue.size() + checkedInQueue.size() << endl;
        cout << " - Early birds: " << earlyBirdCount << endl; // Adjusted output string
        cout << " - Regular: " << regularCount << endl;
        cout << " - Wildcard: " << wildcardCount << endl;
    }

    // Get registration statistics
    void displayRegistrationStatistics() 
    {
        recalculateCounters(); // Ensure counts are up-to-date

        cout << "\n--- Registration Statistics ---" << endl;
        cout << "Players originally loaded from file: " << originalPriorityCount << endl;
        cout << "Players pending check-in (Main Queue): " << mainRegistrationQueue.size() << endl;
        cout << "Players checked in: " << checkedInQueue.size() << endl;
        cout << "Players withdrawn: " << withdrawnPlayersQueue.size() << endl;
        cout << "Players added as replacement: " << replacementCount << endl;
        cout << "Total players managed (sum of all queues): "
            << (mainRegistrationQueue.size() + checkedInQueue.size() + withdrawnPlayersQueue.size() + replacementQueue.size()) << endl;
    }

    // Batch check-in multiple players
    void batchCheckIn(int numberOfPlayers) 
    {
        cout << "\n--- Batch Check-In Process ---" << endl;

        int checkedIn = 0;

        for (int i = 0; i < numberOfPlayers && !mainRegistrationQueue.isEmpty(); i++) {
            checkInPlayer(); // Calls the single checkInPlayer logic
            checkedIn++;
        }

        if (checkedIn < numberOfPlayers) {
            cout << "Note: Only " << checkedIn
                << " players were available for check-in out of " << numberOfPlayers << " requested." << endl;
        } else {
             cout << "Successfully checked in " << checkedIn << " players." << endl;
        }
    }
};

// Function to display menu for the Registration System
void displayRegistrationMenu(TournamentRegistrationSystem& system) 
{
    int choice;
    do {
        cout << "\n" << string(60, '-') << endl;
        cout << "APU E-Sports Championship: Registration Management" << endl;
        cout << string(60, '-') << endl;
        cout << "1. Display all queues\n";
        cout << "2. Batch check-in players\n";
        cout << "3. Withdraw a player\n";
        cout << "4. Add a replacement player\n";
        cout << "5. View registration statistics\n";
        cout << "6. Finish Registration and Start Tournament\n"; // Changed option to proceed
        cout << "Select your choice (between 1-6): ";

        // Input validation loop
        while (!(cin >> choice) || choice < 1 || choice > 6) {
            cout << "Invalid choice. Please enter a number between 1 and 6: ";
            cin.clear(); // Clear error flags
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard bad input
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear rest of line

        switch (choice) {
        case 1:
            system.displayAllQueues();
            break;
        case 2: {
            int number;
            cout << "Enter number of players to check in: ";
            while (!(cin >> number) || number <= 0) {
                cout << "Invalid input. Please enter a positive integer: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            system.batchCheckIn(number);
            break;
        }
        case 3: {
            string playerId;
            cout << "Enter player ID to withdraw: ";
            getline(cin, playerId); 
            system.withdrawPlayer(playerId);
            break;
        }
        case 4: {
            string id, name;
            int rank;
            cout << "Enter replacement player ID (e.g., PL065): ";
            getline(cin, id); 
            cout << "Enter replacement player name: ";
            getline(cin, name);
            cout << "Enter player ranking: ";
            while (!(cin >> rank) || rank <= 0) {
                cout << "Invalid input. Please enter a positive integer for ranking: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            // originalPriority determined by rank, registrationStatus set to "Replacement"
            Player replacement(id, name, rank); 
            system.addReplacementPlayer(replacement);
            break;
        }
        case 5:
            system.displayRegistrationStatistics();
            break;
        case 6:
            cout << "Proceeding to Tournament Simulation." << endl;
            break;
        default: 
            cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 6);
}

int main()
{
    cout << "--- ASIA PACIFIC UNIVERSITY ESPORTS CHAMPIONSHIP ---" << endl;
    cout << "Tournament Registration & Player Queue Management System" << endl;
    cout << string(60, '-') << endl;

    // Seed random number generator ONCE at the start of the program
    srand(static_cast<unsigned int>(time(0)));

    GameResultLogger gameLogger;          // Create Task 4 logger instance
    MatchScheduler scheduler(gameLogger); // Pass logger to Task 1 scheduler

    TournamentRegistrationSystem regSystem; // Instantiate the registration system

    // --- Phase 1: Registration ---
    cout << "\n===== PHASE 1: PLAYER REGISTRATION =====" << endl;
    regSystem.loadPlayersFromFile("updated_player_info.csv"); 
    regSystem.processPriorityRegistrations(); 
    displayRegistrationMenu(regSystem); // User interacts with registration system

    // --- Phase 2: Transition from Registration to Tournament ---
    cout << "\n===== PHASE 2: TOURNAMENT SETUP =====" << endl;

    PlayerQueue* checkedInPlayersQueue = regSystem.getCheckedInPlayersQueue();
    if (checkedInPlayersQueue->isEmpty()) {
        cout << "No players checked in for the tournament. Cannot proceed with tournament simulation. Exiting." << endl;
        return 0;
    }

    // Transfer checked-in players to the MatchScheduler
    int numPlayersAddedToScheduler = 0;
    while (!checkedInPlayersQueue->isEmpty()) {
        Player p = checkedInPlayersQueue->dequeue();
        scheduler.addPlayer(p); // This also initializes player stats in GameResultLogger
        numPlayersAddedToScheduler++;
    }

    if (numPlayersAddedToScheduler == 0) {
        cout << "No players were transferred to the tournament scheduler. Exiting." << endl;
        return 0;
    }

    cout << "Successfully transferred " << numPlayersAddedToScheduler << " checked-in players to the tournament system." << endl;
    
    // Initialize tournament with all successfully added players (sorts them by rank)
    scheduler.initializeTournament(); 

    if (numPlayersAddedToScheduler == 1) {
        cout << "\nOnly one player checked in. This player is the champion by default." << endl;
    } else {
        // --- Phase 3: Group Stage Execution ---
        cout << "\n===== PHASE 3: GROUP STAGE =====" << endl;
        int numGroups = 16; 
        scheduler.runGroupStage(numGroups);
        
        // After group stage, winners are in winnersQueue, they need to be advanced to waitingPlayersQueue
        // to start the main bracket loop. This ensures exactly 32 players enter the main bracket.
        cout << "\n--- Advancing Group Stage Winners to Main Bracket Pool ---" << endl;
        scheduler.advanceToNextRound(); 
    }

    // --- Phase 4: Tournament Simulation (Main Bracket) ---
    cout << "\n===== PHASE 4: TOURNAMENT SIMULATION (MAIN BRACKET) BEGINS =====" << endl;
    int roundNum = 1;
    while (true) {
        if (scheduler.isTournamentOver()) {
            break; // Tournament ends if only one player is left
        }
        cout << "\n<<<<< ROUND " << roundNum << " (Main Bracket) >>>>>" << endl;

        if (!scheduler.createNextRoundPairings()) {
            if (scheduler.isTournamentOver()) break; 
            cout << "No more matches can be scheduled in the main bracket. Tournament might have ended prematurely or unevenly." << endl;
            break; 
        }
        scheduler.playAndProcessMatches();   // Simulate matches, update winners

        if (!scheduler.advanceToNextRound()) {
            if (scheduler.isTournamentOver()) break; 
            cout << "No players advanced to the next round in the main bracket. Tournament might have ended prematurely or unevenly." << endl;
            break; 
        }
        roundNum++;
        
        if (roundNum > 6 && numPlayersAddedToScheduler > 1) { 
            cout << "Warning: Tournament simulation is taking unusually long. Halting to prevent infinite loop." << endl;
            break;
        }
    }

    cout << "\n===== TOURNAMENT SIMULATION COMPLETE =====" << endl;
    Player champion = scheduler.getTournamentWinner();
    if (champion.playerId != "" && champion.playerName != "N/A") { 
        cout << "Champion: " << champion.playerName << " (ID: " << champion.playerId << ", Rank: " << champion.ranking << ")" << endl;
    } else if (numPlayersAddedToScheduler == 1) {
        // Fallback for single player scenario if getTournamentWinner doesn't catch it
        cout << "The sole checked-in player is the champion by default." << endl;
        if (scheduler.getNumInitialPlayers() > 0) { 
            Player solePlayer = scheduler.getInitialPlayer(0);
            cout << "Champion: " << solePlayer.playerName << " (ID: " << solePlayer.playerId << ", Rank: " << solePlayer.ranking << ")" << endl;
        }
    }
    else {
        cout << "No single champion determined, or the tournament ended prematurely." << endl;
    }

    // --- Phase 5: Post-Tournament Reports ---
    cout << "\n===== PHASE 5: POST-TOURNAMENT REPORTS =====" << endl;
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
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

        switch (toupper(reportChoice)) {
            case '1': {
                int count;
                cout << "How many recent matches to display? (e.g., 5): ";
                while (!(cin >> count) || count <= 0) {
                    cout << "Invalid input. Please enter a positive integer: ";
                    cin.clear(); 
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
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
                string playerIdToView; 
                cout << "Enter Player ID to display their full performance record (e.g., PL001): ";
                getline(cin, playerIdToView); 
                gameLogger.displaySinglePlayerPerformance(playerIdToView);
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