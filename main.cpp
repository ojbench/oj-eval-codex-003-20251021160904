#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <unordered_map>
using namespace std;

struct Submission {
    string team_name;
    string problem_name;
    string status;
    int time;
};

struct Team {
    string name;
    int solved_problems = 0;
    int penalty_time = 0;
    map<string, int> wrong_submissions; // problem -> count
    map<string, int> first_ac_time; // problem -> time
    vector<int> solve_times; // sorted in descending order
    vector<Submission> submissions;
};

class ICPCSystem {
private:
    int duration_time;
    bool is_frozen = false;
    map<string, Team> teams;
    vector<string> team_order; // for scoreboard
    vector<Submission> all_submissions;

    void update_team_stats(Team& team, const Submission& sub) {
        team.submissions.push_back(sub);
        
        if (sub.status == "Accepted") {
            // If this is the first AC for this problem
            if (team.first_ac_time.find(sub.problem_name) == team.first_ac_time.end()) {
                team.first_ac_time[sub.problem_name] = sub.time;
                int wrong_count = team.wrong_submissions[sub.problem_name];
                int problem_penalty = 20 * wrong_count + sub.time;
                team.penalty_time += problem_penalty;
                team.solved_problems++;
                team.solve_times.push_back(sub.time);
                sort(team.solve_times.rbegin(), team.solve_times.rend());
            }
        } else {
            // Wrong submission
            team.wrong_submissions[sub.problem_name]++;
        }
    }

    bool compare_teams(const string& a, const string& b) {
        const Team& teamA = teams[a];
        const Team& teamB = teams[b];
        
        if (teamA.solved_problems != teamB.solved_problems) {
            return teamA.solved_problems > teamB.solved_problems;
        }
        if (teamA.penalty_time != teamB.penalty_time) {
            return teamA.penalty_time < teamB.penalty_time;
        }
        for (size_t i = 0; i < min(teamA.solve_times.size(), teamB.solve_times.size()); i++) {
            if (teamA.solve_times[i] != teamB.solve_times[i]) {
                return teamA.solve_times[i] < teamB.solve_times[i];
            }
        }
        return a < b;
    }

    void update_scoreboard() {
        sort(team_order.begin(), team_order.end(), [this](const string& a, const string& b) {
            return compare_teams(a, b);
        });
    }

public:
    void start(int duration) {
        duration_time = duration;
        cout << "[Info]Competition starts.\\n";
    }

    void add_team(const string& team_name) {
        if (teams.find(team_name) != teams.end()) {
            cout << "[Error]Add failed: duplicated team name.\\n";
            return;
        }
        teams[team_name].name = team_name;
        team_order.push_back(team_name);
        cout << "[Info]Add successfully.\\n";
    }

    void submit(const string& team_name, const string& problem_name, const string& status, int time) {
        if (teams.find(team_name) == teams.end()) {
            cout << "[Error]Submit failed: cannot find the team.\\n";
            return;
        }
        
        Submission sub = {team_name, problem_name, status, time};
        all_submissions.push_back(sub);
        update_team_stats(teams[team_name], sub);
        cout << "[Info]Submit successfully.\\n";
    }

    void flush_scoreboard() {
        update_scoreboard();
        cout << "[Info]Flush scoreboard.\\n";
    }

    void freeze() {
        if (is_frozen) {
            cout << "[Error]Freeze failed: scoreboard has been frozen.\\n";
            return;
        }
        is_frozen = true;
        cout << "[Info]Freeze scoreboard.\\n";
    }

    void scroll() {
        if (!is_frozen) {
            cout << "[Error]Scroll failed: scoreboard has not been frozen.\\n";
            return;
        }
        is_frozen = false;
        update_scoreboard();
        cout << "[Info]Scroll scoreboard.\\n";
        
        // Print scoreboard
        for (size_t i = 0; i < team_order.size(); i++) {
            const Team& team = teams[team_order[i]];
            cout << team.name << " " << (i + 1) << " " << team.solved_problems << " " << team.penalty_time << "\\n";
        }
    }

    void query_ranking(const string& team_name) {
        if (teams.find(team_name) == teams.end()) {
            cout << "[Error]Query ranking failed: cannot find the team.\\n";
            return;
        }
        
        if (is_frozen) {
            cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\\n";
        }
        
        // Find ranking
        int ranking = 1;
        for (const string& team : team_order) {
            if (team == team_name) {
                cout << "[" << team_name << "] NOW AT RANKING " << ranking << "\\n";
                return;
            }
            ranking++;
        }
    }

    void query_submission(const string& team_name, const string& problem_name, const string& status) {
        if (teams.find(team_name) == teams.end()) {
            cout << "[Error]Query submission failed: cannot find the team.\\n";
            return;
        }
        
        cout << "[Info]Complete query submission.\\n";
        
        // Find last matching submission
        const vector<Submission>& submissions = teams[team_name].submissions;
        for (auto it = submissions.rbegin(); it != submissions.rend(); it++) {
            const Submission& sub = *it;
            bool problem_match = (problem_name == "ALL") || (sub.problem_name == problem_name);
            bool status_match = (status == "ALL") || (sub.status == status);
            
            if (problem_match && status_match) {
                cout << sub.team_name << " " << sub.problem_name << " " << sub.status << " " << sub.time << "\\n";
                return;
            }
        }
        
        cout << "Cannot find any submission.\\n";
    }

    void end() {
        cout << "[Info]Competition ends.\\n";
    }
};

int main() {
    ICPCSystem system;
    string line;
    
    while (getline(cin, line)) {
        if (line.empty()) continue;
        
        istringstream iss(line);
        string command;
        iss >> command;
        
        if (command == "START") {
            string dummy;
            int duration;
            iss >> dummy >> duration;
            system.start(duration);
        } else if (command == "ADD_TEAM") {
            string team_name;
            iss >> team_name;
            system.add_team(team_name);
        } else if (command == "SUBMIT") {
            string team_name, problem_name, status, dummy;
            int time;
            iss >> team_name >> problem_name >> status >> dummy >> time;
            system.submit(team_name, problem_name, status, time);
        } else if (command == "FLUSH") {
            system.flush_scoreboard();
        } else if (command == "FREEZE") {
            system.freeze();
        } else if (command == "SCROLL") {
            system.scroll();
        } else if (command == "QUERY_RANKING") {
            string team_name;
            iss >> team_name;
            system.query_ranking(team_name);
        } else if (command == "QUERY_SUBMISSION") {
            string team_name, dummy1, problem_cond, dummy2, status_cond;
            iss >> team_name >> dummy1 >> problem_cond >> dummy2 >> status_cond;
            // Extract problem name and status from conditions
            string problem_name = problem_cond.substr(problem_cond.find("=") + 1);
            string status = status_cond.substr(status_cond.find("=") + 1);
            system.query_submission(team_name, problem_name, status);
        } else if (command == "END") {
            system.end();
            break;
        }
    }
    
    return 0;
}
