#include <vector>
using namespace std;

class database_handler {
  public:

  vector<String> lines;

  vector<String> users;

  database_handler() {}

  int getNextAvailableLine() { // Returns next available line to write to that's empty
    int curLine = 1;

    for (String line : lines) {
      line.trim(); // This is really important for the string comparison!!
      if (line[0] == '*') 
        return curLine;
      else 
        curLine++;
    }
    return curLine;
  }

  int findLine(String name) { // Finds the line number of a certain name 
    int curLine = 1;

    for (String line : lines) {
      line.trim(); // This is really important for the string comparison!!
      if (line == name) 
        return curLine;
      else 
        curLine++;
    }
    return -1;
  }

  // void readDatabase() { // for debugging purposes
  //   for (String line : lines) {
  //     USBSerial.println(line);
  //   }
  // }

  void modifyEntry(int targetLine, String word) { // Edit a certain line. Can also add a name to the very end of the database file
    File newFile = SPIFFS.open("/database.txt", "w"); 
    int curLine = 1;
    for (String line : lines) {
      if (curLine++ == targetLine)
        newFile.println(word);
      else
        newFile.println(line);
    }
    if (targetLine == curLine)
      newFile.println(word);
    newFile.close();

    updateLines();
  }

  void clearDatabase() {
    SPIFFS.remove("/database.txt");
    updateLines();
    // File newFile = SPIFFS.open("/database.txt", "w");
    // newFile.close();
  }

  void updateLines() {
    lines.clear();
    File databaseFile = SPIFFS.open("/database.txt", "r");
    
    while (databaseFile.available()) {
      lines.push_back(databaseFile.readStringUntil('\n'));
    }
    databaseFile.close();

    users.clear(); // Creates the array of users that we will need to display to front end
    for (String line : lines) {
      if (line[0] != '*')
        users.push_back(line);
    }
  }
};