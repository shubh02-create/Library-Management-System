/*
    Library Management System
    ----------------------------
    A console-based Library Management System in C++ built with
    object-oriented programming that manages books, members, and
    borrowing records, with persistent file storage.

    Features:
      1. Add a new book
      2. Add a new member
      3. Issue a book to a member
      4. Return a book
      5. Search book by title
      6. Search book by author
      7. Display all books
      8. Display all members
      0. Exit

    Data is stored persistently across three files:
      books.txt   - book catalogue
      members.txt - registered members
      issues.txt  - currently issued books (borrowing records)

    
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <random>
#include <ctime>

using namespace std;

const string BOOKS_FILE   = "books.txt";
const string MEMBERS_FILE = "members.txt";
const string ISSUES_FILE  = "issues.txt";
const char DELIM = '|';

// ----------------------------------------------------------------
// Utility: clear invalid cin state and ignore rest of the line
// ----------------------------------------------------------------
void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            clearInputBuffer();
            cout << "Invalid input. Please enter a valid number.\n";
        } else {
            clearInputBuffer();
            return value;
        }
    }
}

// Case-insensitive substring check, used for search-by-title/author
bool containsIgnoreCase(const string& haystack, const string& needle) {
    string h = haystack, n = needle;
    transform(h.begin(), h.end(), h.begin(), ::tolower);
    transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != string::npos;
}

// ----------------------------------------------------------------
// Book class - encapsulates a single book record (OOP core)
// ----------------------------------------------------------------
class Book {
private:
    int bookId;
    string title;
    string author;
    int totalCopies;
    int availableCopies;

public:
    Book() : bookId(0), totalCopies(0), availableCopies(0) {}

    Book(int id, const string& t, const string& a, int total, int available)
        : bookId(id), title(t), author(a), totalCopies(total), availableCopies(available) {}

    // ---- Getters ----
    int getId() const { return bookId; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }

    // ---- Operations ----
    bool issueCopy() {
        if (availableCopies <= 0) return false;
        availableCopies--;
        return true;
    }

    void returnCopy() {
        if (availableCopies < totalCopies) availableCopies++;
    }

    void display() const {
        cout << left
             << setw(6)  << bookId
             << setw(28) << title
             << setw(20) << author
             << setw(8)  << totalCopies
             << setw(10) << availableCopies << "\n";
    }

    string toFileString() const {
        ostringstream oss;
        oss << bookId << DELIM << title << DELIM << author << DELIM
            << totalCopies << DELIM << availableCopies;
        return oss.str();
    }

    static Book fromFileString(const string& line) {
        stringstream ss(line);
        string field;
        int id, total, avail; string t, a;

        getline(ss, field, DELIM); id = stoi(field);
        getline(ss, field, DELIM); t = field;
        getline(ss, field, DELIM); a = field;
        getline(ss, field, DELIM); total = stoi(field);
        getline(ss, field, DELIM); avail = stoi(field);

        return Book(id, t, a, total, avail);
    }
};

// ----------------------------------------------------------------
// Member class - encapsulates a single library member
// ----------------------------------------------------------------
class Member {
private:
    int memberId;
    string name;
    string contact;

public:
    Member() : memberId(0) {}

    Member(int id, const string& n, const string& c)
        : memberId(id), name(n), contact(c) {}

    int getId() const { return memberId; }
    string getName() const { return name; }
    string getContact() const { return contact; }

    void display() const {
        cout << left
             << setw(6)  << memberId
             << setw(24) << name
             << setw(16) << contact << "\n";
    }

    string toFileString() const {
        ostringstream oss;
        oss << memberId << DELIM << name << DELIM << contact;
        return oss.str();
    }

    static Member fromFileString(const string& line) {
        stringstream ss(line);
        string field;
        int id; string n, c;

        getline(ss, field, DELIM); id = stoi(field);
        getline(ss, field, DELIM); n = field;
        getline(ss, field, DELIM); c = field;

        return Member(id, n, c);
    }
};

// ----------------------------------------------------------------
// IssueRecord - links a book to the member currently borrowing it
// ----------------------------------------------------------------
struct IssueRecord {
    int bookId;
    int memberId;

    string toFileString() const {
        ostringstream oss;
        oss << bookId << DELIM << memberId;
        return oss.str();
    }

    static IssueRecord fromFileString(const string& line) {
        stringstream ss(line);
        string field;
        IssueRecord r;
        getline(ss, field, DELIM); r.bookId = stoi(field);
        getline(ss, field, DELIM); r.memberId = stoi(field);
        return r;
    }
};

// ----------------------------------------------------------------
// Library class - manages books, members, issues, and file I/O
// ----------------------------------------------------------------
class Library {
private:
    vector<Book> books;
    vector<Member> members;
    vector<IssueRecord> issues;

    // ---- File persistence ----
    void saveBooks() {
        ofstream out(BOOKS_FILE, ios::trunc);
        for (const auto& b : books) out << b.toFileString() << "\n";
    }

    void saveMembers() {
        ofstream out(MEMBERS_FILE, ios::trunc);
        for (const auto& m : members) out << m.toFileString() << "\n";
    }

    void saveIssues() {
        ofstream out(ISSUES_FILE, ios::trunc);
        for (const auto& r : issues) out << r.toFileString() << "\n";
    }

    void loadBooks() {
        books.clear();
        ifstream in(BOOKS_FILE);
        string line;
        while (getline(in, line)) if (!line.empty()) books.push_back(Book::fromFileString(line));
    }

    void loadMembers() {
        members.clear();
        ifstream in(MEMBERS_FILE);
        string line;
        while (getline(in, line)) if (!line.empty()) members.push_back(Member::fromFileString(line));
    }

    void loadIssues() {
        issues.clear();
        ifstream in(ISSUES_FILE);
        string line;
        while (getline(in, line)) if (!line.empty()) issues.push_back(IssueRecord::fromFileString(line));
    }

    // ---- Lookup helpers ----
    int findBookIndex(int id) const {
        for (size_t i = 0; i < books.size(); i++)
            if (books[i].getId() == id) return static_cast<int>(i);
        return -1;
    }

    int findMemberIndex(int id) const {
        for (size_t i = 0; i < members.size(); i++)
            if (members[i].getId() == id) return static_cast<int>(i);
        return -1;
    }

    int findIssueIndex(int bookId, int memberId) const {
        for (size_t i = 0; i < issues.size(); i++)
            if (issues[i].bookId == bookId && issues[i].memberId == memberId) return static_cast<int>(i);
        return -1;
    }

    int nextBookId() const {
        int maxId = 100;
        for (const auto& b : books) if (b.getId() >= maxId) maxId = b.getId() + 1;
        return maxId;
    }

    int nextMemberId() const {
        int maxId = 1;
        for (const auto& m : members) if (m.getId() >= maxId) maxId = m.getId() + 1;
        return maxId;
    }

public:
    Library() {
        loadBooks();
        loadMembers();
        loadIssues();
    }

    // ---- Book management ----
    void addBook() {
        string title, author;
        cout << "Enter Book Title: ";
        getline(cin, title);
        cout << "Enter Author Name: ";
        getline(cin, author);
        int copies = readInt("Enter Number of Copies: ");
        if (copies < 1) copies = 1;

        int id = nextBookId();
        books.push_back(Book(id, title, author, copies, copies));
        saveBooks();

        cout << "Book added successfully! Book ID: " << id << "\n";
    }

    void displayAllBooks() const {
        if (books.empty()) {
            cout << "No books in the catalogue.\n";
            return;
        }
        cout << "\n" << left
             << setw(6)  << "ID"
             << setw(28) << "Title"
             << setw(20) << "Author"
             << setw(8)  << "Total"
             << setw(10) << "Available" << "\n";
        cout << string(72, '-') << "\n";
        for (const auto& b : books) b.display();
        cout << "\n";
    }

    void searchByTitle() const {
        string query;
        cout << "Enter title (or part of it) to search: ";
        getline(cin, query);

        bool found = false;
        for (const auto& b : books) {
            if (containsIgnoreCase(b.getTitle(), query)) {
                if (!found) {
                    cout << "\n" << left << setw(6) << "ID" << setw(28) << "Title"
                         << setw(20) << "Author" << setw(8) << "Total" << setw(10) << "Available" << "\n";
                    cout << string(72, '-') << "\n";
                    found = true;
                }
                b.display();
            }
        }
        if (!found) cout << "No books found matching that title.\n";
        cout << "\n";
    }

    void searchByAuthor() const {
        string query;
        cout << "Enter author (or part of name) to search: ";
        getline(cin, query);

        bool found = false;
        for (const auto& b : books) {
            if (containsIgnoreCase(b.getAuthor(), query)) {
                if (!found) {
                    cout << "\n" << left << setw(6) << "ID" << setw(28) << "Title"
                         << setw(20) << "Author" << setw(8) << "Total" << setw(10) << "Available" << "\n";
                    cout << string(72, '-') << "\n";
                    found = true;
                }
                b.display();
            }
        }
        if (!found) cout << "No books found matching that author.\n";
        cout << "\n";
    }

    // ---- Member management ----
    void addMember() {
        string name, contact;
        cout << "Enter Member Name: ";
        getline(cin, name);
        cout << "Enter Contact Number: ";
        getline(cin, contact);

        int id = nextMemberId();
        members.push_back(Member(id, name, contact));
        saveMembers();

        cout << "Member registered successfully! Member ID: " << id << "\n";
    }

    void displayAllMembers() const {
        if (members.empty()) {
            cout << "No members registered.\n";
            return;
        }
        cout << "\n" << left
             << setw(6)  << "ID"
             << setw(24) << "Name"
             << setw(16) << "Contact" << "\n";
        cout << string(46, '-') << "\n";
        for (const auto& m : members) m.display();
        cout << "\n";
    }

    // ---- Issue / Return ----
    void issueBook() {
        int bookId = readInt("Enter Book ID to issue: ");
        int bIdx = findBookIndex(bookId);
        if (bIdx == -1) {
            cout << "Book not found.\n";
            return;
        }

        int memberId = readInt("Enter Member ID: ");
        int mIdx = findMemberIndex(memberId);
        if (mIdx == -1) {
            cout << "Member not found.\n";
            return;
        }

        if (findIssueIndex(bookId, memberId) != -1) {
            cout << "This member already has this book issued.\n";
            return;
        }

        if (!books[bIdx].issueCopy()) {
            cout << "No copies available for this book right now.\n";
            return;
        }

        issues.push_back({bookId, memberId});
        saveBooks();
        saveIssues();

        cout << "Book \"" << books[bIdx].getTitle() << "\" issued to "
             << members[mIdx].getName() << " successfully!\n";
    }

    void returnBook() {
        int bookId = readInt("Enter Book ID to return: ");
        int memberId = readInt("Enter Member ID: ");

        int rIdx = findIssueIndex(bookId, memberId);
        if (rIdx == -1) {
            cout << "No matching issue record found. Check the Book ID and Member ID.\n";
            return;
        }

        int bIdx = findBookIndex(bookId);
        if (bIdx != -1) books[bIdx].returnCopy();

        issues.erase(issues.begin() + rIdx);
        saveBooks();
        saveIssues();

        cout << "Book returned successfully!\n";
    }
};

// ----------------------------------------------------------------
// Display the main menu
// ----------------------------------------------------------------
void showMenu() {
    cout << "\n============ LIBRARY MANAGEMENT SYSTEM ============\n";
    cout << "1. Add New Book\n";
    cout << "2. Add New Member\n";
    cout << "3. Issue Book\n";
    cout << "4. Return Book\n";
    cout << "5. Search Book by Title\n";
    cout << "6. Search Book by Author\n";
    cout << "7. Display All Books\n";
    cout << "8. Display All Members\n";
    cout << "0. Exit\n";
    cout << "=====================================================\n";
}

// ----------------------------------------------------------------
// Main function - menu-driven loop
// ----------------------------------------------------------------
int main() {
    Library library;
    int choice;

    cout << "Welcome to the Library Management System\n";

    do {
        showMenu();
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: library.addBook();          break;
            case 2: library.addMember();        break;
            case 3: library.issueBook();        break;
            case 4: library.returnBook();       break;
            case 5: library.searchByTitle();    break;
            case 6: library.searchByAuthor();   break;
            case 7: library.displayAllBooks();  break;
            case 8: library.displayAllMembers();break;
            case 0: cout << "Exiting... Thank you for using the system!\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);

    return 0;
}
