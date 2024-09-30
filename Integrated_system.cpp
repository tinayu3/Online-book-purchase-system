#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

using namespace std;

// Constants for file names
const string USER_DATA_FILE = "user_data.txt";
const string BOOK_DATA_FILE = "book_data.txt";
const string ORDER_HISTORY_PREFIX = "order_history_";
const string REVIEWS_FILE = "reviews.txt";
const string SESSION_DATA_FILE = "session_data.txt";

// Global mutex for thread synchronization
mutex mtx;

// Base class Buyer
class Buyer {
protected:
    string name;
    int id;
    string address;
    double purchaseAmount;
public:
    Buyer() : purchaseAmount(0.0) {}

    virtual void getBuyName() {
        cout << "Enter your name: ";
        getline(cin, name);
    }

    virtual void getAddress() {
        cout << "Enter your address: ";
        getline(cin, address);
    }

    virtual void getId() {
        cout << "Enter your buyer number (ID): ";
        cin >> id;
        cin.ignore(); // Clear the input buffer
    }

    virtual double getPay() {
        return purchaseAmount;
    }

    virtual void setPay(double amount) = 0; // Pure virtual function
    virtual void display() = 0;             // Pure virtual function

    virtual ~Buyer() {}
};

// Derived class Layfolk (Ordinary Member)
class Layfolk : public Buyer {
public:
    void setPay(double amount) override {
        purchaseAmount = amount; // No discount for ordinary members
    }

    void display() override {
        cout << "\n----- Order Details -----\n";
        cout << "Buyer Type: Ordinary Member\n";
        cout << "Name: " << name << endl;
        cout << "Buyer ID: " << id << endl;
        cout << "Address: " << address << endl;
        cout << "Total Amount Payable: $" << fixed << setprecision(2) << purchaseAmount << endl;
    }
};

// Derived class Member (Gold Member)
class Member : public Buyer {
protected:
    int starLevel; // 1 to 5 stars
public:
    void getStarLevel() {
        cout << "Enter your membership star level (1-5): ";
        cin >> starLevel;
        while (starLevel < 1 || starLevel > 5) {
            cout << "Invalid star level. Please enter a value between 1 and 5: ";
            cin >> starLevel;
        }
        cin.ignore(); // Clear the input buffer
    }

    void setPay(double amount) override {
        double discountRate;
        switch (starLevel) {
            case 5:
                discountRate = 0.70;
                break;
            case 4:
                discountRate = 0.80;
                break;
            case 3:
                discountRate = 0.85;
                break;
            case 2:
                discountRate = 0.90;
                break;
            case 1:
                discountRate = 0.95;
                break;
            default:
                discountRate = 1.0;
        }
        purchaseAmount = amount * discountRate;
    }

    void display() override {
        cout << "\n----- Order Details -----\n";
        cout << "Buyer Type: Gold Member\n";
        cout << "Name: " << name << endl;
        cout << "Buyer ID: " << id << endl;
        cout << "Star Level: " << starLevel << " Star\n";
        cout << "Address: " << address << endl;
        cout << "Total Amount Payable after Discount: $" << fixed << setprecision(2) << purchaseAmount << endl;
    }
};

// Derived class HonoredGuest (Diamond Member)
class HonoredGuest : public Buyer {
protected:
    double discountRate; // e.g., 0.60 means 40% off
public:
    void getDiscountRate() {
        cout << "Enter your special discount rate (e.g., enter 0.60 for 40% off): ";
        cin >> discountRate;
        while (discountRate <= 0 || discountRate > 1) {
            cout << "Invalid discount rate. Please enter a value between 0 and 1: ";
            cin >> discountRate;
        }
        cin.ignore(); // Clear the input buffer
    }

    void setPay(double amount) override {
        purchaseAmount = amount * discountRate;
    }

    void display() override {
        cout << "\n----- Order Details -----\n";
        cout << "Buyer Type: Diamond Member\n";
        cout << "Name: " << name << endl;
        cout << "Buyer ID: " << id << endl;
        cout << "Address: " << address << endl;
        cout << "Special Discount Rate: " << (1 - discountRate) * 100 << "% off\n";
        cout << "Total Amount Payable after Discount: $" << fixed << setprecision(2) << purchaseAmount << endl;
    }
};

// Class for Books
class Book {
private:
    int bookID;
    string title;
    string author;
    double price;
    int stockQuantity;
    double ratingSum;
    int ratingCount;
public:
    Book(int id, string t, string a, double p, int sq)
        : bookID(id), title(t), author(a), price(p), stockQuantity(sq), ratingSum(0.0), ratingCount(0) {}

    int getBookID() const { return bookID; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    double getPrice() const { return price; }
    int getStockQuantity() const { return stockQuantity; }
    double getAverageRating() const {
        if (ratingCount == 0) return 0.0;
        return ratingSum / ratingCount;
    }

    void setStockQuantity(int quantity) {
        stockQuantity = quantity;
    }

    void addRating(double rating) {
        ratingSum += rating;
        ratingCount++;
    }

    void displayBook() const {
        cout << left << setw(5) << bookID
             << setw(25) << title
             << setw(20) << author
             << "$" << fixed << setprecision(2) << price
             << setw(10) << " Stock: " << stockQuantity
             << " Rating: " << fixed << setprecision(1) << getAverageRating() << "/5"
             << endl;
    }
};

// Class for User (for authentication)
class User {
private:
    string username;
    string password;
    int id;
    bool isLoggedIn;
public:
    User(string uname, string pwd, int uid) : username(uname), password(pwd), id(uid), isLoggedIn(false) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    int getId() const { return id; }
    void setLoggedIn(bool status) { isLoggedIn = status; }
    bool getLoggedIn() const { return isLoggedIn; }
};

// Class for Admin (inherits User)
class Admin : public User {
public:
    Admin(string uname, string pwd, int uid) : User(uname, pwd, uid) {}

    // Administrative functions
    void addBook(vector<Book>& books) {
        int id, stock;
        string title, author;
        double price;
        cout << "Enter new book ID: ";
        cin >> id;
        cin.ignore();
        cout << "Enter book title: ";
        getline(cin, title);
        cout << "Enter author name: ";
        getline(cin, author);
        cout << "Enter price: ";
        cin >> price;
        cout << "Enter stock quantity: ";
        cin >> stock;
        cin.ignore();
        books.push_back(Book(id, title, author, price, stock));
        cout << "Book added successfully.\n";
    }

    void removeBook(vector<Book>& books) {
        int id;
        cout << "Enter the ID of the book to remove: ";
        cin >> id;
        cin.ignore();
        auto it = find_if(books.begin(), books.end(), [id](const Book& b) {
            return b.getBookID() == id;
        });
        if (it != books.end()) {
            books.erase(it);
            cout << "Book removed successfully.\n";
        } else {
            cout << "Book not found.\n";
        }
    }

    void updateBookStock(vector<Book>& books) {
        int id, newStock;
        cout << "Enter the ID of the book to update stock: ";
        cin >> id;
        cout << "Enter new stock quantity: ";
        cin >> newStock;
        cin.ignore();
        auto it = find_if(books.begin(), books.end(), [id](Book& b) {
            return b.getBookID() == id;
        });
        if (it != books.end()) {
            it->setStockQuantity(newStock);
            cout << "Stock updated successfully.\n";
        } else {
            cout << "Book not found.\n";
        }
    }
};

// Function prototypes
void showWelcomeMessage();
void displayBookList(const vector<Book>& books);
void addBooksToCart(vector<pair<Book, int>>& cart, vector<Book>& books);
double calculateTotal(const vector<pair<Book, int>>& cart);
void saveOrderToFile(Buyer* buyer, const vector<pair<Book, int>>& cart);
void viewCart(const vector<pair<Book, int>>& cart);
void removeBookFromCart(vector<pair<Book, int>>& cart);
void updateBookQuantity(vector<pair<Book, int>>& cart);
void userRegistration(map<string, User>& users);
User userLogin(map<string, User>& users);
void viewOrderHistory(const User& user);
void adminMenu(Admin& admin, vector<Book>& books);
void applyCoupon(Buyer* buyer);
void processPayment(Buyer* buyer);
void saveUsersToFile(const map<string, User>& users);
void loadUsersFromFile(map<string, User>& users);
void saveBooksToFile(const vector<Book>& books);
void loadBooksFromFile(vector<Book>& books);
void searchBooks(const vector<Book>& books);
void filterBooks(const vector<Book>& books);
void rateBook(vector<Book>& books);
void addToWishlist(vector<Book>& wishlist, const vector<Book>& books);
void viewWishlist(const vector<Book>& wishlist);
void sendEmailNotification(const User& user, const string& message);
void handleGiftOption(vector<pair<Book, int>>& cart);
void returnOrRefund(vector<Book>& books);
void startUserSession(User& user);
void saveSessionData(const User& user);
void loadSessionData(map<string, User>& users);
string encryptData(const string& data);
string decryptData(const string& data);

// Exception classes
class AuthenticationError : public exception {
public:
    const char* what() const noexcept override {
        return "Authentication failed.";
    }
};

class BookNotFoundError : public exception {
public:
    const char* what() const noexcept override {
        return "Book not found.";
    }
};

class OutOfStockError : public exception {
public:
    const char* what() const noexcept override {
        return "Requested quantity not available.";
    }
};

// Entry point of the program
int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed for random numbers

    // Load users and books from files
    map<string, User> users;
    loadUsersFromFile(users);
    vector<Book> books;
    loadBooksFromFile(books);

    // Ensure admin user exists
    users["admin"] = User("admin", "admin123", 0); // Admin user

    vector<pair<Book, int>> shoppingCart;
    vector<Book> wishlist;
    Buyer* buyer = nullptr;

    showWelcomeMessage();

    // User authentication
    int choice;
    cout << "1. Register\n2. Login\nChoose an option: ";
    cin >> choice;
    cin.ignore();

    User currentUser("", "", -1);

    try {
        if (choice == 1) {
            userRegistration(users);
            saveUsersToFile(users);
            currentUser = userLogin(users);
        } else if (choice == 2) {
            currentUser = userLogin(users);
        } else {
            cout << "Invalid choice. Exiting.\n";
            return 0;
        }
    } catch (AuthenticationError& e) {
        cout << e.what() << endl;
        return 0;
    }

    // Start user session
    thread userSession(startUserSession, ref(currentUser));
    userSession.detach();

    // Check if the user is an admin
    if (currentUser.getUsername() == "admin") {
        Admin admin(currentUser.getUsername(), currentUser.getPassword(), currentUser.getId());
        adminMenu(admin, books);
        saveBooksToFile(books);
        return 0;
    }

    // Proceed with shopping
    int buyerType = 0;
    int id = currentUser.getId();

    // Determine buyer type based on ID
    if (id >= 1 && id <= 100) {
        buyerType = 1; // Gold Member
    } else if (id >= 200 && id <= 300) {
        buyerType = 2; // Diamond Member
    } else if (id >= 1000 && id <= 2000) {
        buyerType = 3; // Ordinary Member
    } else {
        cout << "Invalid buyer ID. Exiting the system.\n";
        return 0;
    }

    // Create appropriate buyer object
    switch (buyerType) {
        case 1:
            buyer = new Member();
            break;
        case 2:
            buyer = new HonoredGuest();
            break;
        case 3:
            buyer = new Layfolk();
            break;
        default:
            cout << "Error determining buyer type.\n";
            return 0;
    }

    buyer->getId();
    buyer->getBuyName();
    buyer->getAddress();

    // Additional details based on buyer type
    if (buyerType == 1) {
        static_cast<Member*>(buyer)->getStarLevel();
    } else if (buyerType == 2) {
        static_cast<HonoredGuest*>(buyer)->getDiscountRate();
    }

    // Main menu loop
    bool exitProgram = false;
    while (!exitProgram) {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Browse Books\n2. Search Books\n3. View Cart\n4. View Wishlist\n5. View Order History\n6. Checkout\n7. Logout\nChoose an option: ";
        int mainChoice;
        cin >> mainChoice;
        cin.ignore();

        switch (mainChoice) {
            case 1:
                displayBookList(books);
                addBooksToCart(shoppingCart, books);
                break;
            case 2:
                searchBooks(books);
                break;
            case 3:
                viewCart(shoppingCart);
                break;
            case 4:
                viewWishlist(wishlist);
                break;
            case 5:
                viewOrderHistory(currentUser);
                break;
            case 6:
                if (shoppingCart.empty()) {
                    cout << "Your cart is empty.\n";
                    break;
                }
                // Gift Option
                handleGiftOption(shoppingCart);
                // Apply coupon
                applyCoupon(buyer);
                // Calculate total amount
                double totalAmount;
                totalAmount = calculateTotal(shoppingCart);
                buyer->setPay(totalAmount);
                // Display order details
                buyer->display();
                // Process payment
                processPayment(buyer);
                // Save order to file
                saveOrderToFile(buyer, shoppingCart);
                // Update stock quantities
                for (const auto& item : shoppingCart) {
                    auto it = find_if(books.begin(), books.end(), [item](Book& b) {
                        return b.getBookID() == item.first.getBookID();
                    });
                    if (it != books.end()) {
                        int newStock = it->getStockQuantity() - item.second;
                        it->setStockQuantity(newStock);
                    }
                }
                // Save books to file
                saveBooksToFile(books);
                // Clear cart
                shoppingCart.clear();
                // Send email notification
                sendEmailNotification(currentUser, "Your order has been placed successfully!");
                break;
            case 7:
                exitProgram = true;
                break;
            default:
                cout << "Invalid choice. Try again.\n";
                break;
        }
    }

    // Clean up
    delete buyer;

    cout << "\nThank you for visiting the Online Bookstore!\n";

    return 0;
}

// Function Definitions

void showWelcomeMessage() {
    cout << "**********************************\n";
    cout << "* Welcome to the Online Bookstore *\n";
    cout << "**********************************\n\n";
}

void displayBookList(const vector<Book>& books) {
    cout << "\nAvailable Books:\n";
    cout << left << setw(5) << "ID"
         << setw(25) << "Title"
         << setw(20) << "Author"
         << "Price" << setw(15) << " Stock"
         << " Rating\n";
    cout << "---------------------------------------------------------------------------------\n";
    for (const auto& book : books) {
        book.displayBook();
    }
}

void addBooksToCart(vector<pair<Book, int>>& cart, vector<Book>& books) {
    char choice = 'y';
    while (tolower(choice) == 'y') {
        int bookID = 0;
        int quantity = 0;
        cout << "Enter the ID of the book you want to add to your cart: ";
        cin >> bookID;
        cin.ignore(); // Clear the input buffer

        auto it = find_if(books.begin(), books.end(), [bookID](const Book& b) {
            return b.getBookID() == bookID;
        });

        if (it != books.end()) {
            cout << "Enter quantity: ";
            cin >> quantity;
            cin.ignore();
            if (quantity > it->getStockQuantity()) {
                cout << "Only " << it->getStockQuantity() << " copies available.\n";
                quantity = it->getStockQuantity();
            }
            cart.push_back(make_pair(*it, quantity));
            cout << "\"" << it->getTitle() << "\" has been added to your cart.\n";
        } else {
            cout << "Book with ID " << bookID << " not found.\n";
        }
        cout << "Do you want to add another book to your cart? (y/n): ";
        cin >> choice;
        cin.ignore();
    }
}

double calculateTotal(const vector<pair<Book, int>>& cart) {
    double total = 0.0;
    cout << "\nBooks in your cart:\n";
    for (const auto& item : cart) {
        cout << "- " << item.first.getTitle() << " x" << item.second << " ($" << item.first.getPrice() << " each)\n";
        total += item.first.getPrice() * item.second;
    }
    cout << "Total amount before discount: $" << fixed << setprecision(2) << total << endl;
    return total;
}

void saveOrderToFile(Buyer* buyer, const vector<pair<Book, int>>& cart) {
    ofstream outFile("order_details.txt", ios::app);
    if (!outFile) {
        cout << "Error opening file to save order details.\n";
        return;
    }

    outFile << "----- Order Details -----\n";
    outFile << "Name: " << buyer->getPay() << endl;
    outFile << "Buyer ID: " << buyer->getPay() << endl;
    outFile << "Address: " << buyer->getPay() << endl;

    outFile << "Books Purchased:\n";
    for (const auto& item : cart) {
        outFile << "- " << item.first.getTitle() << " by " << item.first.getAuthor()
                << " x" << item.second << " ($" << fixed << setprecision(2) << item.first.getPrice() << " each)\n";
    }
    outFile << "Total Amount Paid: $" << fixed << setprecision(2) << buyer->getPay() << "\n\n";
    outFile.close();

    // Save order history per user
    string filename = ORDER_HISTORY_PREFIX + to_string(buyer->getPay()) + ".txt";
    ofstream historyFile(filename, ios::app);
    if (historyFile) {
        historyFile << "----- Order Details -----\n";
        historyFile << "Name: " << buyer->getPay() << endl;
        historyFile << "Buyer ID: " << buyer->getPay() << endl;
        historyFile << "Address: " << buyer->getPay() << endl;

        historyFile << "Books Purchased:\n";
        for (const auto& item : cart) {
            historyFile << "- " << item.first.getTitle() << " by " << item.first.getAuthor()
                        << " x" << item.second << " ($" << fixed << setprecision(2) << item.first.getPrice() << " each)\n";
        }
        historyFile << "Total Amount Paid: $" << fixed << setprecision(2) << buyer->getPay() << "\n\n";
        historyFile.close();
    }
}

void viewCart(const vector<pair<Book, int>>& cart) {
    if (cart.empty()) {
        cout << "Your cart is empty.\n";
        return;
    }
    cout << "\nYour Shopping Cart:\n";
    for (const auto& item : cart) {
        cout << "- " << item.first.getTitle() << " x" << item.second << " ($" << item.first.getPrice() << " each)\n";
    }
}

void removeBookFromCart(vector<pair<Book, int>>& cart) {
    if (cart.empty()) {
        cout << "Your cart is empty.\n";
        return;
    }
    int bookID;
    cout << "Enter the ID of the book to remove from your cart: ";
    cin >> bookID;
    cin.ignore();

    auto it = find_if(cart.begin(), cart.end(), [bookID](const pair<Book, int>& item) {
        return item.first.getBookID() == bookID;
    });

    if (it != cart.end()) {
        cart.erase(it);
        cout << "Book removed from your cart.\n";
    } else {
        cout << "Book not found in your cart.\n";
    }
}

void updateBookQuantity(vector<pair<Book, int>>& cart) {
    if (cart.empty()) {
        cout << "Your cart is empty.\n";
        return;
    }
    int bookID, newQuantity;
    cout << "Enter the ID of the book to update quantity: ";
    cin >> bookID;
    cin.ignore();

    auto it = find_if(cart.begin(), cart.end(), [bookID](pair<Book, int>& item) {
        return item.first.getBookID() == bookID;
    });

    if (it != cart.end()) {
        cout << "Enter new quantity: ";
        cin >> newQuantity;
        cin.ignore();
        it->second = newQuantity;
        cout << "Quantity updated.\n";
    } else {
        cout << "Book not found in your cart.\n";
    }
}

void userRegistration(map<string, User>& users) {
    string username, password;
    int id;
    cout << "Enter a username: ";
    getline(cin, username);
    if (users.find(username) != users.end()) {
        cout << "Username already exists. Try logging in.\n";
        return;
    }
    cout << "Enter a password: ";
    getline(cin, password);
    cout << "Enter your buyer ID: ";
    cin >> id;
    cin.ignore();

    users[username] = User(username, password, id);
    cout << "Registration successful.\n";
}

User userLogin(map<string, User>& users) {
    string username, password;
    cout << "Enter your username: ";
    getline(cin, username);
    cout << "Enter your password: ";
    getline(cin, password);

    auto it = users.find(username);
    if (it != users.end() && it->second.getPassword() == password) {
        cout << "Login successful.\n";
        it->second.setLoggedIn(true);
        return it->second;
    } else {
        throw AuthenticationError();
    }
}

void viewOrderHistory(const User& user) {
    string filename = ORDER_HISTORY_PREFIX + to_string(user.getId()) + ".txt";
    ifstream historyFile(filename);
    if (!historyFile) {
        cout << "No order history found.\n";
        return;
    }
    cout << "\nYour Order History:\n";
    string line;
    while (getline(historyFile, line)) {
        cout << line << endl;
    }
    historyFile.close();
}

void adminMenu(Admin& admin, vector<Book>& books) {
    int choice;
    do {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Add Book\n2. Remove Book\n3. Update Book Stock\n4. View Books\n5. Exit\nChoose an option: ";
        cin >> choice;
        cin.ignore();
        switch (choice) {
            case 1:
                admin.addBook(books);
                break;
            case 2:
                admin.removeBook(books);
                break;
            case 3:
                admin.updateBookStock(books);
                break;
            case 4:
                displayBookList(books);
                break;
            case 5:
                cout << "Exiting Admin Menu.\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
                break;
        }
    } while (choice != 5);
}

void applyCoupon(Buyer* buyer) {
    char choice;
    cout << "Do you have a discount coupon? (y/n): ";
    cin >> choice;
    cin.ignore();
    if (tolower(choice) == 'y') {
        string couponCode;
        cout << "Enter coupon code: ";
        getline(cin, couponCode);
        // For simplicity, let's say "SAVE10" gives a 10% discount
        if (couponCode == "SAVE10") {
            double currentPay = buyer->getPay();
            buyer->setPay(currentPay * 0.90);
            cout << "Coupon applied! You get a 10% discount.\n";
        } else {
            cout << "Invalid coupon code.\n";
        }
    }
}

void processPayment(Buyer* buyer) {
    cout << "\nProcessing payment...\n";
    cout << "Payment of $" << fixed << setprecision(2) << buyer->getPay() << " successful.\n";
}

void saveUsersToFile(const map<string, User>& users) {
    ofstream userFile(USER_DATA_FILE);
    if (!userFile) {
        cout << "Error saving user data.\n";
        return;
    }
    for (const auto& pair : users) {
        userFile << pair.second.getUsername() << " " << pair.second.getPassword() << " " << pair.second.getId() << endl;
    }
    userFile.close();
}

void loadUsersFromFile(map<string, User>& users) {
    ifstream userFile(USER_DATA_FILE);
    if (!userFile) {
        cout << "No user data found. Starting fresh.\n";
        return;
    }
    string username, password;
    int id;
    while (userFile >> username >> password >> id) {
        users[username] = User(username, password, id);
    }
    userFile.close();
}

void saveBooksToFile(const vector<Book>& books) {
    ofstream bookFile(BOOK_DATA_FILE);
    if (!bookFile) {
        cout << "Error saving book data.\n";
        return;
    }
    for (const auto& book : books) {
        bookFile << book.getBookID() << "|"
                 << book.getTitle() << "|"
                 << book.getAuthor() << "|"
                 << book.getPrice() << "|"
                 << book.getStockQuantity() << "|"
                 << book.getAverageRating() << endl;
    }
    bookFile.close();
}

void loadBooksFromFile(vector<Book>& books) {
    ifstream bookFile(BOOK_DATA_FILE);
    if (!bookFile) {
        cout << "No book data found. Using default books.\n";
        // Initialize default books
        books = {
            Book(1, "The Great Gatsby", "F. Scott Fitzgerald", 10.99, 10),
            Book(2, "1984", "George Orwell", 8.99, 5),
            Book(3, "To Kill a Mockingbird", "Harper Lee", 12.50, 8),
            Book(4, "Pride and Prejudice", "Jane Austen", 9.99, 7),
            Book(5, "The Catcher in the Rye", "J.D. Salinger", 11.20, 6),
            Book(6, "The Hobbit", "J.R.R. Tolkien", 15.00, 4),
            Book(7, "Moby Dick", "Herman Melville", 13.45, 9),
            Book(8, "War and Peace", "Leo Tolstoy", 20.00, 3),
            Book(9, "The Odyssey", "Homer", 14.25, 12),
            Book(10, "Hamlet", "William Shakespeare", 9.75, 15)
        };
        return;
    }
    string line;
    while (getline(bookFile, line)) {
        stringstream ss(line);
        string token;
        vector<string> tokens;
        while (getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        if (tokens.size() >= 5) {
            int id = stoi(tokens[0]);
            string title = tokens[1];
            string author = tokens[2];
            double price = stod(tokens[3]);
            int stock = stoi(tokens[4]);
            books.push_back(Book(id, title, author, price, stock));
        }
    }
    bookFile.close();
}

void searchBooks(const vector<Book>& books) {
    cout << "Enter keyword to search for books: ";
    string keyword;
    getline(cin, keyword);
    transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
    vector<Book> results;
    for (const auto& book : books) {
        string title = book.getTitle();
        string author = book.getAuthor();
        transform(title.begin(), title.end(), title.begin(), ::tolower);
        transform(author.begin(), author.end(), author.begin(), ::tolower);
        if (title.find(keyword) != string::npos || author.find(keyword) != string::npos) {
            results.push_back(book);
        }
    }
    if (results.empty()) {
        cout << "No books found matching \"" << keyword << "\".\n";
    } else {
        cout << "\nSearch Results:\n";
        for (const auto& book : results) {
            book.displayBook();
        }
    }
}

void filterBooks(const vector<Book>& books) {
    // Implement filtering functionality based on price, rating, etc.
    // This function is a placeholder for future development.
}

void rateBook(vector<Book>& books) {
    int bookID;
    double rating;
    cout << "Enter the ID of the book you want to rate: ";
    cin >> bookID;
    cin.ignore();
    auto it = find_if(books.begin(), books.end(), [bookID](Book& b) {
        return b.getBookID() == bookID;
    });
    if (it != books.end()) {
        cout << "Enter your rating (1-5): ";
        cin >> rating;
        cin.ignore();
        if (rating >= 1.0 && rating <= 5.0) {
            it->addRating(rating);
            cout << "Thank you for rating \"" << it->getTitle() << "\".\n";
        } else {
            cout << "Invalid rating. Please enter a value between 1 and 5.\n";
        }
    } else {
        cout << "Book not found.\n";
    }
}

void addToWishlist(vector<Book>& wishlist, const vector<Book>& books) {
    int bookID;
    cout << "Enter the ID of the book to add to your wishlist: ";
    cin >> bookID;
    cin.ignore();
    auto it = find_if(books.begin(), books.end(), [bookID](const Book& b) {
        return b.getBookID() == bookID;
    });
    if (it != books.end()) {
        wishlist.push_back(*it);
        cout << "\"" << it->getTitle() << "\" has been added to your wishlist.\n";
    } else {
        cout << "Book not found.\n";
    }
}

void viewWishlist(const vector<Book>& wishlist) {
    if (wishlist.empty()) {
        cout << "Your wishlist is empty.\n";
        return;
    }
    cout << "\nYour Wishlist:\n";
    for (const auto& book : wishlist) {
        book.displayBook();
    }
}

void sendEmailNotification(const User& user, const string& message) {
    cout << "\nSending email to " << user.getUsername() << "...\n";
    cout << "Email content: " << message << endl;
    // Simulate email sending delay
    this_thread::sleep_for(chrono::seconds(1));
    cout << "Email sent successfully.\n";
}

void handleGiftOption(vector<pair<Book, int>>& cart) {
    char choice;
    cout << "Do you want to purchase any item as a gift? (y/n): ";
    cin >> choice;
    cin.ignore();
    if (tolower(choice) == 'y') {
        // For simplicity, this function is a placeholder
        cout << "Gift option selected. Proceeding with gift purchase.\n";
    }
}

void returnOrRefund(vector<Book>& books) {
    // Implement return or refund functionality
    // This function is a placeholder for future development.
}

void startUserSession(User& user) {
    unique_lock<mutex> lock(mtx);
    user.setLoggedIn(true);
    saveSessionData(user);
    cout << "User session started for " << user.getUsername() << endl;
}

void saveSessionData(const User& user) {
    ofstream sessionFile(SESSION_DATA_FILE, ios::app);
    if (sessionFile) {
        sessionFile << user.getUsername() << " " << user.getLoggedIn() << endl;
        sessionFile.close();
    }
}

void loadSessionData(map<string, User>& users) {
    ifstream sessionFile(SESSION_DATA_FILE);
    if (sessionFile) {
        string username;
        bool loggedIn;
        while (sessionFile >> username >> loggedIn) {
            auto it = users.find(username);
            if (it != users.end()) {
                it->second.setLoggedIn(loggedIn);
            }
        }
        sessionFile.close();
    }
}

string encryptData(const string& data) {
    // Simple encryption (placeholder)
    string encrypted = data;
    for (char& c : encrypted) {
        c += 3;
    }
    return encrypted;
}

string decryptData(const string& data) {
    // Simple decryption (placeholder)
    string decrypted = data;
    for (char& c : decrypted) {
        c -= 3;
    }
    return decrypted;
}
