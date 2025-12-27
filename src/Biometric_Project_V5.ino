#include <WiFi.h>
#include <vector>
#include <sqlite3.h>
#include <SPIFFS.h>
#include <Keypad_MCP23008.h>
#include <Hash.h>
#include <WebServer.h>
#include <map>
#include <GxEPD2_4C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <QRCode.h>




// Define your WiFi credentials
/*char *ssid = "Proximus-Home-5C68";
char *password = "w7wkbpfd7zfzh";*/

char *ssid = "Galaxy S21 FE 5G 25F4";
char *password = "galaxyglodi";

bool hasRows = false;
std::map <String, int> sessionTokenMap;

WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Define your SQLite database file
sqlite3* db;

// Define the keypad

const byte I2C_ADDRESS = 0x20;
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','4','7','*'},
  {'2','5','8','0'},
  {'3','6','9','#'},
  {'A','B','C','D'}
};
byte rowPins[ROWS] = {7, 6, 5, 4}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {3, 2, 1, 0}; // Connect to the column pinouts of the keypad

Keypad_MCP23008 keypad = Keypad_MCP23008(rowPins, colPins, ROWS, COLS);
GxEPD2_4C < GxEPD2_437c, GxEPD2_437c::HEIGHT / 4 > display(GxEPD2_437c(/*CS=*/ D5, /*DC=*/ D3, /*RST=*/ D2, /*BUSY=*/ D4)); // Waveshare 4.37" 4-color
 


/*--void handleRoot();
void handleProducts();
void handleAddProduct();
void handleEditProduct();
void handleLogin();
void handleSignIn();
void handleUserPage();
void handleMarkAsPaid();
void handleOrderDetails();
void handleDeleteProductByID();
void handleWelcomePage();
void storeSessionToken(const String& sessionToken, int clientID);
String generateSessionToken();
bool isNumeric(const String& str);
int getClientIDFromSessionToken(const String& sessionToken);
int callbackUserPage(void *data, int argc, char **argv, char **azColName);*/

void setup() {
  keypad.begin(I2C_ADDRESS, makeKeymap(keys));
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");
  pinMode(D6, OUTPUT);
  digitalWrite(D6, LOW);


  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected to WiFi");
   
 
  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  
  // Open the SQLite database in SPIFFS
  sqlite3_initialize();
  if (db_open("/spiffs/snacks.db", &db)) {
    Serial.println("Failed to open SQLite database");
    return;
  }
  keypad.addEventListener(keypadEvent);
  
   display.init(115200, true, 2, false);
  
  display.setTextColor(GxEPD_BLACK);

   server.on("/home", HTTP_GET, handleRoot);
    server.on("/products", HTTP_GET, handleProducts);
    server.on("/products", HTTP_POST, handleProducts);
    server.on("/add_product", HTTP_GET, handleAddProduct); // Route for handling the confirmation of adding a product
    server.on("/add_product", HTTP_POST, handleAddProduct);
    server.on("/edit_product", HTTP_GET, handleEditProduct);
    server.on("/edit_product", HTTP_POST, handleEditProduct);
    server.on("/login", HTTP_GET, handleLogin);
    server.on("/login", HTTP_POST, handleLogin);
    server.on("/signin", HTTP_GET, handleSignIn);
    server.on("/signin", HTTP_POST, handleSignIn);
    server.on("/userpage", HTTP_GET, handleUserPage);
    server.on("/userpage", HTTP_POST, handleUserPage);
    server.on("/mark-as-paid", HTTP_POST, handleMarkAsPaid);
    server.on("/order-details", handleOrderDetails);
    server.on("/delete_product", HTTP_GET, handleDeleteProductByID);
    server.on("/delete_product", HTTP_POST, handleDeleteProductByID);
    server.on("/welcome", HTTP_GET, handleWelcomePage);
    server.on("/adminpage",HTTP_GET,handleAdminPage); // Add the handleAdminPage route

    server.begin();
    Serial.println("HTTP server started");
    
    IPAddress ip = WiFi.localIP(); // Use the obtained IP address
    if (!MDNS.begin("snackbarESP32")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("MDNS responder started");
  timeClient.begin();
  timeClient.setTimeOffset(3600); // Set time offset (in seconds), e.g., 3600 for UTC+1 (Central European Time)

}


void loop() {
  // Wait for user input
  menu();
  server.handleClient();
  while (true) {
    server.handleClient();
    char key = keypad.getKey();
    if (key != NO_KEY) {
      // User chooses between ordering (A) or restocking (B)
      if (key == 'A') {
         int idClient = authenticateCustomer(); // Get the authenticated ID_Client
         if (idClient != -1) { // Proceed if authentication is successful
            orderProducts(idClient); // Pass the authenticated ID_Client to orderProducts()
         }
        break;
      } else if (key == 'B') {
        restockProducts();
        break;
      }
    }
  }
}

// Function to open the SQLite database
int db_open(const char *filename, sqlite3 **db) {
  int rc = sqlite3_open(filename, db);
  if (rc != SQLITE_OK) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
    return 1;
  } else {
    Serial.println("Opened database successfully");
  }
  return 0;
}

void printlnScreen(const char* text) {
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  digitalWrite(D6, LOW);
  display.setFont(&FreeMonoBold12pt7b);
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);  
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 20);
    display.println(text);
  } while (display.nextPage());
  digitalWrite(D6, HIGH);
}
// Function to display the menu options
void menu() {

    // Function to display the menu options
    String menuText = "Welcome to the Snack Bar!\n\nPlease choose an option:\nA. Order Snacks\nB. Restock Snacks\nVisit http://snackbaresp32/home\n";
    menuText += "\nAvailable Snacks:\n";
     // Concatenate product details
      sqlite3_stmt *stmt;
      const char *query = "SELECT ID_Product, Name, UnitPrice, Stocks FROM Product";
      int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        printlnScreen("Failed to prepare query");
        return;
      }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      // Fetch product details
      int id = sqlite3_column_int(stmt, 0);
      const unsigned char *name = sqlite3_column_text(stmt, 1);
      float price = sqlite3_column_double(stmt, 2);
      int stocks = sqlite3_column_int(stmt, 3);
      // Concatenate product information
    if (stocks>0)
    {menuText += String("ID ") + String(id) + ": " + String(reinterpret_cast<const char*>(name)) + ", " + String(price) + "$, stock: " + String(stocks) + "\n";}
    }
    sqlite3_finalize(stmt);
    printlnScreen(menuText.c_str());    
}

// Function to authenticate the customer
int authenticateCustomer() {
    // Prompt the user for their Registration Number (ID_Client)
    String message="Follow these 2 steps to authenticate\n";
    message +="1. Enter your ID followed by #\n";
    message +="2. Enter your PinCode followed by #\n";
    message+="\nIf you don't have an account yet\nvisit http://snackbaresp32/login\n";
    message+="If you can't access the website,\n";
    message+="Press # to return to the menu\n";
    message+="and select option B";

    printlnScreen(message.c_str());

    String idClientStr = readFromKeypad(); // Implement the function to read from keypad
    int idClient = idClientStr.toInt();

    // Query the database to verify if the provided ID_Client exists
    sqlite3_stmt *stmt;
    const char *query = "SELECT ID_Client FROM Customer WHERE ID_Client = ?";
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Serial.println("Failed to prepare authentication query");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, idClient);

    // Execute the query
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        // ID_Client not found
        printlnScreen("Invalid Unique ID.\nPlease try again.\nTo create a new account, visit\nhttp://snackbaresp32/login");

        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);

    // Prompt the user for their pin code (Password)
    int attemptsLeft = 3;
    while (attemptsLeft > 0) {
        String pinCode = readFromKeypad(); // Implement the function to read from keypad
        if (pinCode.length() > 0) {
            pinCode.remove(pinCode.length() - 1);
        }
        String hashedPinCode = sha1(pinCode); // Implement the function to hash using SHA-1

        // Query the database to verify the provided ID_Client and hashed Password
        const char *query = "SELECT ID_Client FROM Customer WHERE ID_Client = ? AND Password = ?";
        int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            Serial.println("Failed to prepare authentication query");
            return -1;
        }
        
        sqlite3_bind_int(stmt, 1, idClient);
        sqlite3_bind_text(stmt, 2, hashedPinCode.c_str(), -1, SQLITE_TRANSIENT);

        // Execute the query
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            // Authentication successful
            sqlite3_finalize(stmt);
            return idClient;
        } else {
            // Authentication failed
         attemptsLeft--;
         if (attemptsLeft>0)
         { String message = "Invalid pin code. Please try again. \nYou have " + String(attemptsLeft) + " attempts remaining.\nPlease enter your pin code (Password):";
          printlnScreen(message.c_str());
         }
        }
    }
    sqlite3_finalize(stmt);

    return -1; // Authentication failed after all attempts
}


// Function to order products
void orderProducts(int idClient) {
  std::vector<int> orderedProducts;
  std::vector<int> orderedQuantities;

  // Display welcome message and prompt
  String productDetails;
 const char *query = "SELECT Name FROM Customer WHERE ID_Client = ?";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    printlnScreen("Failed to prepare query");
    return;
  }
  sqlite3_bind_int(stmt, 1, idClient);
  if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    const unsigned char *name = sqlite3_column_text(stmt, 0);
    productDetails = "Welcome ";
    productDetails += reinterpret_cast<const char*>(name);
    productDetails += ":)\nWhat would you like to order?:\n";
    
  } else {
    Serial.println("Failed to retrieve customer's name");
    return;
  }
  sqlite3_finalize(stmt);  
  // Query the database for available products

  query = "SELECT ID_Product, Name, UnitPrice, Stocks FROM Product";
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    Serial.println("Failed to prepare query");
    return;
  }

  // Concatenate product details
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    // Fetch product details
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    float price = sqlite3_column_double(stmt, 2);
    int stocks = sqlite3_column_int(stmt, 3);
    // Concatenate product information
    if (stocks>0)
    {productDetails += String("ID ") + String(id) + ": " + String(reinterpret_cast<const char*>(name)) + ", " + String(price) + "$, stock: " + String(stocks) + "\n";}
  }
  sqlite3_finalize(stmt);

  // Display available products
 
  // Prompt user for product selection
  productDetails += "\nEnter product ID followed by # \nOr press B to end your order";
  printlnScreen(productDetails.c_str());

  String productIdString = "";
  bool no_error=true;
  char key;
  while (true) {
    if (no_error){key = readCharFromKeypad();}
    if (key != NO_KEY) {
      if (key == '#') {
        // Process selected product
        int productId = productIdString.toInt();
        // Fetch product details from the database
        sqlite3_stmt *stmt;
        const char *query = "SELECT Name, UnitPrice, Stocks FROM Product WHERE ID_Product = ?";
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
          Serial.println("Failed to prepare query");
          return;
        }
        sqlite3_bind_int(stmt, 1, productId);
        if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
          // Product found, display details
          const unsigned char *name = sqlite3_column_text(stmt, 0);
          float price = sqlite3_column_double(stmt, 1);
          int stocks = sqlite3_column_int(stmt, 2);
          
          // Display selected product details
          String selectedProductDetails;
          selectedProductDetails += "Product selected\n";
          selectedProductDetails += String("Name: ") + reinterpret_cast<const char*>(name) + "\n";
          selectedProductDetails += String("Price: ") + String(price) + "\n";
          selectedProductDetails += String("Stock: ") + String(stocks) + "\n";
          selectedProductDetails +="\nEnter the desired quantity followed by #";
          selectedProductDetails +="\nAdd more products to your order? \nPress A for Yes\nOr Press B to end your order\n";
          printlnScreen(selectedProductDetails.c_str());
    
          String quantityString = readFromKeypad();
          int quantity = quantityString.toInt();
          if (quantity <= 0) {
            printlnScreen("Invalid quantity");
            key='A';
            no_error=false;
            continue;
          }
          if (quantity > stocks) {
            printlnScreen("Sorry, insufficient stock available.");
            key='A';
            no_error=false;
            continue;
          }
          // Store the order details
          orderedProducts.push_back(productId);
          orderedQuantities.push_back(quantity);
          productIdString = "";
        } else {
          // Invalid product ID
          printlnScreen("Invalid product ID. Please try again.");
          key='A';
          no_error=false;
          continue;
        }
        sqlite3_finalize(stmt);
      } else if (key == 'A') {
          // User wants to add more products
          printlnScreen(productDetails.c_str());
          no_error=true;
          productIdString = "";
      } else if (key == 'B') {
        // User finished ordering
        break;
      } else {
        productIdString += key;
      }
    }
  }

  // Calculate and display order total
  int orderTotal = 0;
  String orderSummary;
  orderSummary += "Your order:\n";
  for (size_t i = 0; i < orderedProducts.size(); ++i) {
    int productId = orderedProducts[i];
    int quantity = orderedQuantities[i];
    sqlite3_stmt *stmt;
    const char *query = "SELECT Name, UnitPrice FROM Product WHERE ID_Product = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
      Serial.println("Failed to prepare query");
      return;
    }
    sqlite3_bind_int(stmt, 1, productId);
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      const unsigned char *name = sqlite3_column_text(stmt, 0);
      float price = sqlite3_column_double(stmt, 1);
      int subtotal = quantity * price;
      orderTotal += subtotal;
      orderSummary += String("  -") + reinterpret_cast<const char*>(name);
      orderSummary += String("    Quantity: ") + String(quantity) ;
      orderSummary += String("    Price: ") + String(subtotal) + "$\n";
    }
    sqlite3_finalize(stmt);
  }
  orderSummary += String("Order total: ") + String(orderTotal) + "$\n";

  orderSummary +="Confirm order? \nPress A for Yes \nPress B for no ";
  printlnScreen(orderSummary.c_str());

  char confirmation = readCharFromKeypad();
  if (confirmation == 'A') {
    // Process order and update database
    sqlite3_stmt *stmt;
    timeClient.update();
    unsigned long currentEpoch = timeClient.getEpochTime();
    time_t rawtime = currentEpoch;
    struct tm *timeinfo;
    char buffer[11];
    timeinfo = localtime(&rawtime);
    sprintf(buffer, "%04d-%02d-%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    String currentDate = String(buffer);

    String insertOrderQuery = "INSERT INTO Orders (DateC, Status, ID_Client) VALUES ('" + currentDate + "', 'NoPaid', ?)";
    int rc = sqlite3_prepare_v2(db, insertOrderQuery.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
      Serial.println("Failed to prepare order insertion query");
      return;
    }


  // Bind the ID_Client value to the prepared statement
  sqlite3_bind_int(stmt, 1, idClient);

  // Execute the query
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
      Serial.println("Failed to insert order");
      sqlite3_finalize(stmt); // Finalize the statement
      Serial.println(sqlite3_errmsg(db)); // Print the error message
      return;
  }
  
    int orderId = sqlite3_last_insert_rowid(db);

    for (size_t i = 0; i < orderedProducts.size(); ++i) {
      int productId = orderedProducts[i];
      int quantity = orderedQuantities[i];
      const char *updateStockQuery = "UPDATE Product SET Stocks = Stocks - ? WHERE ID_Product = ?";
      rc = sqlite3_prepare_v2(db, updateStockQuery, -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        Serial.println("Failed to prepare stock update query");
        return;
      }
      sqlite3_bind_int(stmt, 1, quantity);
      sqlite3_bind_int(stmt, 2, productId);
      rc = sqlite3_step(stmt);
      if (rc != SQLITE_DONE) {
        Serial.println("Failed to update stocks");
        sqlite3_finalize(stmt);
        return;
      }
      sqlite3_finalize(stmt);

      const char *insertOrderItemQuery = "INSERT INTO Product_Order (ID_Order, ID_Product, Quantity) VALUES (?, ?, ?)";
      rc = sqlite3_prepare_v2(db, insertOrderItemQuery, -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        Serial.println("Failed to prepare order item insertion query");
        return;
      }
      sqlite3_bind_int(stmt, 1, orderId);
      sqlite3_bind_int(stmt, 2, productId);
      sqlite3_bind_int(stmt, 3, quantity);
      rc = sqlite3_step(stmt);
      if (rc != SQLITE_DONE) {
        Serial.println("Failed to insert order item");
        sqlite3_finalize(stmt);
        return;
      }
      sqlite3_finalize(stmt);
    }
    printlnScreen("Order confirmed.");
  } else {
    printlnScreen("Order canceled.");
  }
}


// Function to read input from the keypad
String readFromKeypad() {
  String input = "";
  char key;
  do {
    key = keypad.getKey();
    if (key != NO_KEY) {
      input += key;
    }
  } while (key != '#'); // Wait for the user to enter '#' to confirm input
  return input;
}

// Function to read a single character input from the keypad
char readCharFromKeypad() {
  char key;
  do {
    key = keypad.getKey();
  } while (key == NO_KEY); // Wait for the user to press a key
  return key;
}

// Function to handle keypad events
void keypadEvent(KeypadEvent key) {
    server.handleClient();
  if (keypad.getState() == PRESSED) {
    digitalWrite(D6, LOW);
    if (key != NO_KEY) {
      switch (key) {
        case 'A':
        case 'B':
        case 'C':
        case 'D':
          Serial.println(key);   
          break;
        case '#':
          Serial.println();
          break;
        default:
          Serial.print(key);
          break;
      }
    }
    digitalWrite(D6, HIGH);
  }
}

// Function to restock products
void restockProducts() {
  // Implement restocking functionality here
}


String epochToDateString(unsigned long epochTime) {
  time_t rawtime = epochTime;
  struct tm *timeinfo;
  char buffer[12];

  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);

  return String(buffer);
}


    //-----------------------------------//
   //-------------HOME PAGE-------------//
  //-----------------------------------//
void handleRoot() {
    File file = SPIFFS.open("/home.html", "r");
    
    if (!file) {
        Serial.println("Failed to open index.html file");
        return;
    }

    server.streamFile(file, "text/html");
    file.close();
}



int callback(void *data, int argc, char **argv, char **azColName) {
  hasRows = true; // Set the flag to true
  int *Client_ID = (int *)data;
  *Client_ID = atoi(argv[0]); // Extract the name from the result  
  return 0;
}
    //-----------------------------------//
   //-------------LOGIN PAGE------------//
  //-----------------------------------//
void handleLogin() {
    if (server.method() == HTTP_GET) {
        // Read the login.html file from SPIFFS
        File file = SPIFFS.open("/login.html", "r");
        if (!file) {
            Serial.println("Failed to open login.html file");
            server.send(500, "text/plain", "Internal Server Error");
            return;
        }

        // Serve the login page HTML content
        server.streamFile(file, "text/html");
        file.close();
    } else if (server.method() == HTTP_POST) {
        // Handle login form submission
        String registrationNumber = server.arg("registration_number"); // Change "email" to "registration_number"
        String hashedPinCode = sha1(server.arg("pin_code")); // Change "password" to "pin_code"

        if (registrationNumber.equals("admin") && hashedPinCode.equals(sha1("IOT_Project"))) {
          // Admin clientID
          int adminID = 0;

          // Generate a session token (you can use any method to generate a unique token)
          String sessionToken = generateSessionToken();

          // Store the session token and admin status in the server's memory or database
          storeSessionToken(sessionToken, adminID);

          // Redirect to the admin page with the session token
          server.sendHeader("Location", "/adminpage?token=" + sessionToken, true);
          server.send(302, "text/plain", "");
          return;
      }
        // Query the database to check if the registration number exists
        char *errorMsg;
        int clientID;
        String query = "SELECT ID_Client FROM Customer WHERE ID_Client = '" + registrationNumber + "'";
        int result = sqlite3_exec(db, query.c_str(), callback, (void*)&clientID, &errorMsg);
        if (result != SQLITE_OK) {
            Serial.print("Database error: ");
            Serial.println(errorMsg);
            sqlite3_free(errorMsg);
            return;
        }

        // If the registration number exists, check if the pin code is correct
        if (hasRows) {
            // Reset hasRows flag
            hasRows = false;
            
            // Query the database to check if the registration number and pin code match
            query = "SELECT ID_Client FROM Customer WHERE ID_Client = '" + registrationNumber + "' AND Password = '" + hashedPinCode + "'";
            result = sqlite3_exec(db, query.c_str(), callback, (void*)&clientID, &errorMsg);
            if (result != SQLITE_OK) {
                Serial.print("Database error: ");
                Serial.println(errorMsg);
                sqlite3_free(errorMsg);
                return;
            }

            // If the registration number and pin code match, redirect to user page with the Client_ID
            if (hasRows) {
                // Generate a session token (you can use any method to generate a unique token)
                String sessionToken = generateSessionToken();

                // Store the session token and corresponding Client_ID in the server's memory or database
                storeSessionToken(sessionToken, clientID);

                // Redirect to the user page with the session token
                server.sendHeader("Location", "/userpage?token=" + sessionToken, true);
                server.send(302, "text/plain", "");
            } else {
                // Display error message for incorrect pin code and reload login page
                String errorMsg = "<script>alert('Incorrect pin code. Please try again.'); window.location.href = '/login';</script>";
                server.send(200, "text/html", errorMsg);
            }
        } else {
            // Display error message for registration number not found in database
            String errorMsg = "<script>alert('Registration number not found. Please check your registration number.'); window.location.href = '/login';</script>";
            server.send(200, "text/html", errorMsg);
        }
    } else {
        // Invalid request method
        server.send(405, "text/plain", "Method Not Allowed");
    }
}



    //-----------------------------------//
   //-------------SIGNUP PAGE-------------//
  //-----------------------------------//
void handleSignIn() {
    if (server.method() == HTTP_GET) {
        File file = SPIFFS.open("/signin.html", "r");
        if (!file) {
            Serial.println("Failed to open signin.html file");
            server.send(404, "text/plain", "File Not Found");
            return;
        }

        String contentType = "text/html";
        if (server.streamFile(file, contentType) != file.size()) {
            Serial.println("Failed to send signin.html file");
        }
        file.close();
    } else if (server.method() == HTTP_POST) {
        // Handle form submission
        String name = server.arg("name");
        String surname = server.arg("surname");
        String email = server.arg("email");
        String pinCode = server.arg("pin_code"); // Change "password" to "pin_code"

        // Validate email format
        if (email.indexOf('@') == -1) {
            String errorMsg = "<script>alert('Invalid email address format.'); window.location.href = '/signin';</script>";
            server.send(200, "text/html", errorMsg);
            return;
        }

        // Validate pin code complexity
        if (!isNumeric(pinCode) || pinCode.length() < 4) {
            String errorMsg = "<script>alert('Pin code must be minimum 4 digits long and contain only numeric characters.'); window.location.href = '/signin';</script>";
            server.send(200, "text/html", errorMsg);
            return;
        }

        // Hash the pin code before storing it in the database
        String hashedPinCode = sha1(pinCode);
        if (hashedPinCode.length() == 0) {
            String errorMsg = "<script>alert('Failed to hash pin code.'); window.location.href = '/signin';</script>";
            server.send(200, "text/html", errorMsg);
            return;
        }

        // Check if the email already exists in the database
        String query = "SELECT COUNT(*) FROM Customer WHERE Email = '" + email + "'";
        sqlite3_stmt *statement;
        int result = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
        if (result == SQLITE_OK) {
            if (sqlite3_step(statement) == SQLITE_ROW) {
                int count = sqlite3_column_int(statement, 0);
                if (count > 0) {
                    String errorMsg = "<script>alert('Email address already exists.'); window.location.href = '/signin';</script>";
                    server.send(200, "text/html", errorMsg);
                    sqlite3_finalize(statement);
                    return;
                }
            }
            sqlite3_finalize(statement);
        } else {
            String errorMsg = "<script>alert('Internal Server Error.'); window.location.href = '/signin';</script>";
            server.send(200, "text/html", errorMsg);
            return;
        }

        // Insert the new customer into the database
        query = "INSERT INTO Customer (Name, Surname, Email, Password) VALUES ('" + name + "', '" + surname + "', '" + email + "', '" + hashedPinCode + "');";
        if (sqlite3_exec(db, query.c_str(), NULL, 0, NULL) != SQLITE_OK) {
            String errorMsg = "<script>alert('Error inserting customer into database.'); window.location.href = '/signin';</script>";
            server.send(200, "text/html", errorMsg);
            return;
        }

        // Store the session token and corresponding Client_ID in the server's memory or database
        int clientID; // Declare clientID variable
        query = "SELECT ID_Client FROM Customer WHERE Email = '" + email + "'";
        result = sqlite3_exec(db, query.c_str(), callback, (void*)&clientID, NULL);
        if (result != SQLITE_OK) {
            String errorMsg = "<script>alert('Error retrieving client ID.'); window.location.href = '/signin';</script>";
            server.send(200, "text/html", errorMsg);
            return;
        }

        // Generate a session token
        String sessionToken = generateSessionToken();

        // Store the session token and corresponding Client_ID in the server's memory or database
        storeSessionToken(sessionToken, clientID);

        // Prepare the response HTML with JavaScript for opening the welcome page in a new window
        String redirectScript = "<script>window.open('/welcome?token=" + sessionToken + "', '_blank'); window.close();</script>";
        server.send(200, "text/html", redirectScript);
    } else {
        // Invalid request method
        server.send(405, "text/plain", "Method Not Allowed");
    }
}



bool isNumeric(const String& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        if (!isdigit(str.charAt(i))) {
            return false;
        }
    }
    return true;
}

    //------------------------------------//
   //-------WELCOME USER PAGE------------//
  //------------------------------------//
void handleWelcomePage() {
    // Get the session token from the URL parameter
    String sessionToken = server.arg("token");

    // Get the client ID from the session token
    int clientID = getClientIDFromSessionToken(sessionToken);

    // Fetch user details from the database using the clientID
    String userName = ""; // Placeholder for the user's name fetched from the database

    // Execute SQL query to fetch user's name based on clientID
    String query = "SELECT Name FROM Customer WHERE ID_Client = " + String(clientID);
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
    if (result == SQLITE_OK) {
        if (sqlite3_step(statement) == SQLITE_ROW) {
            userName = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
        }
        sqlite3_finalize(statement);
    } else {
        // Handle SQL error
        Serial.println("Error executing SQL query to fetch user's name");
    }

    // Serve the welcome page HTML content
    File file = SPIFFS.open("/welcome.html", "r");
    if (!file) {
        Serial.println("Failed to open welcome.html file");
        return;
    }

    String welcomePageContent = "";
    while (file.available()) {
        welcomePageContent += char(file.read());
    }
    file.close();

    // Replace placeholders in the HTML content with actual data
    welcomePageContent.replace("[USERNAME]", userName);
    welcomePageContent.replace("[CLIENT_ID]", String(clientID));

    server.send(200, "text/html", welcomePageContent);
}



    //-----------------------------------//
   //-------------USER PAGE-------------//
  //-----------------------------------//

void handleUserPage() {
    String sessionToken = server.arg("token");
    int clientID = getClientIDFromSessionToken(sessionToken);

    String query = "SELECT Name FROM Customer WHERE ID_Client = " + String(clientID);
    char *errorMsg;
    String name;

    int result = sqlite3_exec(db, query.c_str(), callbackUserPage, (void*)&name, &errorMsg);
    if (result != SQLITE_OK) {
        Serial.print("Database error: ");
        Serial.println(errorMsg);
        sqlite3_free(errorMsg);
        return;
    }

    // Serve the userpage.html file from SPIFFS
    File userPageFile = SPIFFS.open("/userpage.html", "r");
    if (!userPageFile) {
        Serial.println("Failed to open userpage.html file");
        return;
    }

    String content = userPageFile.readString();
    userPageFile.close();

    // Replace [USERNAME] with the actual name
    content.replace("[USERNAME]", name);

    // Fetch orders grouped by their IDs
    query = "SELECT o.ID_Order, SUM(po.Quantity * p.UnitPrice) AS TotalPrice, o.DateC, o.Status FROM Orders o JOIN Product_Order po ON o.ID_Order = po.ID_Order JOIN Product p ON po.ID_Product = p.ID_Product WHERE o.ID_Client = ? GROUP BY o.ID_Order";
    sqlite3_stmt *statement;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
        sqlite3_bind_int(statement, 1, clientID);
        String orderDetails;
        while (sqlite3_step(statement) == SQLITE_ROW) {
            int orderID = sqlite3_column_int(statement, 0);
            double totalPrice = sqlite3_column_double(statement, 1);
            String orderDate = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
            String status = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 3)));
            orderDetails += "<tr><td>" + String(orderID) + "</td><td>" + orderDate + "</td><td>" + String(totalPrice, 2) + "€</td><td>" + status + "</td><td><a href=\"#\" onclick=\"showOrderDetails(" + String(orderID) + ")\">Show Details</a></td><td><input type=\"checkbox\" name=\"orderCheckbox\" value=\"" + String(orderID) + "\"></td></tr>";
        }
        sqlite3_finalize(statement);
        // Insert order details into the content
        content.replace("<!-- Order details will be dynamically inserted here -->", orderDetails);
    }

    // Replace the incorrect button links
    content.replace("<button onclick=\"window.location.href='/login'\">Login</button>", "<button onclick=\"window.location.href='/login'\">Logout</button>");
    content.replace("window.location.href='/home'", "window.location.href='/userpage?token=" + sessionToken + "'");
    content.replace("window.location.href='/products'", "window.location.href='/products?token=" + sessionToken + "'");

    server.send(200, "text/html", content);
}




void handleOrderDetails() {
    String orderIDStr = server.arg("orderID");
    int orderID = orderIDStr.toInt();
    String content;

    // Serve the orderdetails.html file from SPIFFS
    File orderDetailsFile = SPIFFS.open("/orderdetails.html", "r");
    if (!orderDetailsFile) {
        Serial.println("Failed to open orderdetails.html file");
        server.send(500, "text/plain", "Internal Server Error");
        return;
    }

    content = orderDetailsFile.readString();
    orderDetailsFile.close();

    // Prepare the SQL query to fetch order details from the database
    String query = "SELECT p.Name AS snackName, po.Quantity AS quantity, p.UnitPrice AS unitPrice, (po.Quantity * p.UnitPrice) AS totalPrice FROM Product p JOIN Product_Order po ON p.ID_Product = po.ID_Product WHERE po.ID_Order = " + String(orderID);
    sqlite3_stmt *statement;

    // Execute the SQL query and fetch the results
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
        String orderDetails;
        while (sqlite3_step(statement) == SQLITE_ROW) {
            // Extract snack name, quantity, unit price, and total price from the current row
            String snackName = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
            int quantity = sqlite3_column_int(statement, 1);
            double unitPrice = sqlite3_column_double(statement, 2);
            double totalPrice = sqlite3_column_double(statement, 3);

            // Construct a table row for the current order detail
            orderDetails += "<tr><td>" + snackName + "</td><td>" + String(quantity) + "</td><td>" + String(unitPrice) + "€</td><td>" + String(totalPrice) + "€</td></tr>";
        }

        // Insert order details into the content
        content.replace("<!-- Order details will be dynamically inserted here -->", orderDetails);

        // Finalize the SQL statement
        sqlite3_finalize(statement);

        // Send the HTTP response with the HTML content
        server.send(200, "text/html", content);
    } else {
        // If the SQL query execution fails, send an error response
        server.send(500, "text/plain", "Internal Server Error");
    }
}


int callbackUserPage(void *data, int argc, char **argv, char **azColName) {
    String *name = (String *)data;
    *name = String(argv[0]); // Extract the name from the result
    return 0;
}
void handleMarkAsPaid() {
    String requestBody = server.arg("plain");
    StaticJsonDocument<200> doc;
    deserializeJson(doc, requestBody);

    JsonArray orders = doc["orders"];
    for (JsonVariant order : orders) {
        int orderID = order.as<int>();

        // Update the status of the order to "Paid" in the database
        String query = "UPDATE Orders SET Status = 'Paid' WHERE ID_Order = " + String(orderID);
        char *errorMsg;
        int result = sqlite3_exec(db, query.c_str(), NULL, NULL, &errorMsg);
        if (result != SQLITE_OK) {
            Serial.print("Database error: ");
            Serial.println(errorMsg);
            sqlite3_free(errorMsg);
            server.send(500, "text/plain", "Internal Server Error");
            return;
        }
    }

    server.send(200, "text/plain", "OK");
}


  void storeSessionToken(const String& sessionToken, int clientID) {
      sessionTokenMap[sessionToken] = clientID;
  }

String generateSessionToken() {
    // Here, we'll generate a random string of characters
    const String validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const int tokenLength = 32;
    String sessionToken = "";

    for (int i = 0; i < tokenLength; ++i) {
        int index = random(validChars.length());
        sessionToken += validChars[index];
    }

    return sessionToken;
}
int getClientIDFromSessionToken(const String& sessionToken) {
    // Check if session token exists in the map
    if (sessionTokenMap.find(sessionToken) != sessionTokenMap.end()) {
        // Return the corresponding Client_ID
        return sessionTokenMap[sessionToken];
    } else {
        // If session token doesn't exist, return -1 (or any appropriate default value)
        return -1;
    }
}


    //-----------------------------------//
   //-------------SNACK PAGE-------------//
  //-----------------------------------//

void handleProducts() {
    if (server.method() == HTTP_GET) {
        String sessionToken = server.arg("token");
        int clientID = getClientIDFromSessionToken(sessionToken);

        // Serve the products.html file from SPIFFS
        File productsFile = SPIFFS.open("/products.html", "r");
        if (!productsFile) {
            Serial.println("Failed to open products.html file");
            return;
        }

        String content = productsFile.readString();
        productsFile.close();

        // Replace [TOKEN] with the actual token value
        content.replace("INSERT_TOKEN_HERE", sessionToken);

        // Replace the button links with proper token passing
        if (clientID != -1) {
            content.replace("<button onclick=\"window.location.href='/home'\">Home</button>", "<button onclick=\"window.location.href='/userpage?token=" + sessionToken + "'\">Home</button>");
            content.replace("<button onclick=\"window.location.href='/login'\">Login</button>", "<button onclick=\"window.location.href='/login'\">Logout</button>");
        }
        content.replace("<button onclick=\"window.location.href='/products'\">Snacks</button>", "<button onclick=\"window.location.href='/products?token=" + sessionToken + "'\">Snacks</button>");


        // Replace <!-- Snack data will be dynamically inserted here --> with actual product details
        String productDetails;
        sqlite3_stmt *statement;
        if (sqlite3_prepare_v2(db, "SELECT ID_Product, Name, Stocks, UnitPrice FROM Product", -1, &statement, NULL) == SQLITE_OK) {
            while (sqlite3_step(statement) == SQLITE_ROW) {
                int productId = sqlite3_column_int(statement, 0);
                String name = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
                int quantity = sqlite3_column_int(statement, 2);
                double price = sqlite3_column_double(statement, 3);

                productDetails += "<tr><td>" + name + "</td><td>" + String(quantity) + "</td><td>" + String(price, 2) + "</td>";
                productDetails += "<td><button onclick=\"editRow(this, " + String(productId) + ", '" + sessionToken + "')\"><i class=\"fas fa-pencil-alt\"></i></button></td>";
                productDetails += "<td><button onclick=\"deleteRow(this, " + String(productId) + ", '" + sessionToken + "')\"><i class=\"fas fa-trash\"></i></button></td></tr>";
            }
            sqlite3_finalize(statement);
        }
        content.replace("<!-- Snack data will be dynamically inserted here -->", productDetails);

        server.send(200, "text/html", content);
    }
}

    //-----------------------------------//
   //-----------ADD SNACK PAGE----------//
  //-----------------------------------//


void handleAddProduct() {
      String sessionToken = server.arg("token");
      int clientID = getClientIDFromSessionToken(sessionToken);
      
    if (server.method() == HTTP_GET) {
        // Serve the add_product.html file from SPIFFS
        File addProductFile = SPIFFS.open("/add_product.html", "r");
        if (!addProductFile) {
            Serial.println("Failed to open add_product.html file");
            return;
        }

        String content = addProductFile.readString();
        addProductFile.close();

        // Replace the button links with proper token passing
         if (clientID != -1) {
            content.replace("<button onclick=\"window.location.href='/home'\">Home</button>", "<button onclick=\"window.location.href='/userpage?token=" + sessionToken + "'\">Home</button>");
            content.replace("<button onclick=\"window.location.href='/login'\">Login</button>", "<button onclick=\"window.location.href='/logout'\">Logout</button>");
        }
        content.replace("<button onclick=\"window.location.href='/products'\">Snacks</button>", "<button onclick=\"window.location.href='/products?token=" + sessionToken + "'\">Snacks</button>");


        server.send(200, "text/html", content);
    } else if (server.method() == HTTP_POST) {
        // Confirm the addition of the new product

        String productName = server.arg("productName");
        int quantity = server.arg("quantity").toInt();
        float price = server.arg("price").toFloat();

        // Validate input data
        if (productName.length() == 0 || quantity < 0 || price < 0) {
            server.send(400, "text/plain", "Invalid input data.");
            return;
        }

        // Add the new product to the database
        String query = "INSERT INTO Product (Name, UnitPrice, Stocks) VALUES ('" + productName + "', " + String(price) + ", " + String(quantity) + ");";

        if (sqlite3_exec(db, query.c_str(), NULL, 0, NULL) != SQLITE_OK) {
            server.send(500, "text/plain", "Error adding product to database.");
            return;
        }

        // Redirect back to the products page after adding the product
        server.sendHeader("Location", "/products?token=" + sessionToken);
        server.send(302, "text/plain", ""); // 302 status code for redirection
    }
}



void handleEditProduct() {

      String sessionToken = server.arg("token");
        int clientID = getClientIDFromSessionToken(sessionToken);
    if (server.method() == HTTP_GET) {
       
        // If it's a GET request, redirect to the products page
        server.sendHeader("Location", "/products?token=" + sessionToken, true);
        server.send(302, "text/plain", ""); // 302 status code for redirection
    } else if (server.method() == HTTP_POST && server.uri() == "/edit_product") {
        // Extract parameters from the request
        int productId = server.arg("id").toInt();
        String newName = server.arg("name");
        int newQuantity = server.arg("quantity").toInt();
        float newPrice = server.arg("price").toFloat();

        // Validate data
        if (newQuantity < 0 || newPrice < 0) {
            server.send(400, "text/plain", "Invalid data provided.");
            return;
        }

        // Update the product in the database
        String query = "UPDATE Product SET Name = '" + newName + "', Stocks = " + String(newQuantity) + ", UnitPrice = " + String(newPrice) + " WHERE ID_Product= " + String(productId);
        Serial.println("Executing query: " + query); // Debugging information

        int result = sqlite3_exec(db, query.c_str(), NULL, 0, NULL);
        if (result != SQLITE_OK) {
            Serial.println("Error updating product. SQLite error code: " + String(result)); // Debugging information
            server.send(500, "text/plain", "Error updating product.");
            return;
        }

        Serial.println("Product updated successfully."); // Debugging information

        // Redirect back to the products page after updating the product
        server.sendHeader("Location", "/products?token=" + sessionToken);
        server.send(302, "text/plain", ""); // 302 status code for redirection
    }
}



void handleDeleteProductByID() {
    if (server.method() == HTTP_GET) {
        // If it's a GET request, redirect to the products page
        server.sendHeader("Location", "/products?token=" + server.arg("token"), true);
        server.send(302, "text/plain", ""); // 302 status code for redirection
    } else if (server.method() == HTTP_POST) {
        // Extract product ID from the request
        int productId = server.arg("id").toInt();

        // Construct the DELETE query
        String query = "DELETE FROM Product WHERE ID_Product = " + String(productId);

        // Execute the query
        int result = sqlite3_exec(db, query.c_str(), NULL, 0, NULL);
        if (result != SQLITE_OK) {
            Serial.println("Error deleting product. SQLite error code: " + String(result)); // Debugging information
            server.send(500, "text/plain", "Error deleting product.");
            return;
        }

        Serial.println(query); // Debugging information

        // Redirect back to the products page after deleting the product
        server.sendHeader("Location", "/products?token=" + server.arg("token"));
        server.send(302, "text/plain", ""); // 302 status code for redirection
    }
}

void handleExecute() {
  if (!server.hasArg("query")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  String query = server.arg("query");

  if (sqlite3_exec(db, query.c_str(), NULL, NULL, NULL) != SQLITE_OK) {
    server.send(500, "text/plain", "Error executing query");
    return;
  }

  server.send(200, "text/plain", "Query executed successfully");
}

void handleAdminPage() {
    // Check if the request method is GET and the client has a valid session token
    if (server.method() == HTTP_GET) {
        // Get the client ID from the session token
        String sessionToken = server.arg("token");
        int clientID = getClientIDFromSessionToken(sessionToken);

        // Only allow access if the client ID is 0 (admin)
        if (clientID == 0) {
            // Serve the adminpage.html file from SPIFFS
            File adminPageFile = SPIFFS.open("/adminpage.html", "r");
            if (!adminPageFile) {
                Serial.println("Failed to open adminpage.html file");
                server.send(500, "text/plain", "Internal Server Error");
                return;
            }

            String content = adminPageFile.readString();
            adminPageFile.close();

            // Generate the current date

             String currentDate= server.arg("date");
        
            // Parse the current date to extract month and year
            String selectedMonthStr = currentDate.substring(5, 7); // Extract month (MM)
            String selectedYearStr = currentDate.substring(0, 4); // Extract year (YYYY)

            int selectedMonth = selectedMonthStr.toInt();
            int selectedYear = selectedYearStr.toInt();
              Serial.println("Month:" + String(selectedMonth));
            Serial.println("Year:" + String(selectedYear));

            // Handle fetching orders for the current month
            String query = "SELECT Orders.ID_Order, Orders.DateC, Orders.Status, Customer.Name, Customer.Surname, Customer.Email, SUM(po.Quantity * p.UnitPrice) AS TotalPrice FROM Orders JOIN Customer ON Orders.ID_Client = Customer.ID_Client JOIN Product_Order po ON Orders.ID_Order = po.ID_Order JOIN Product p ON po.ID_Product = p.ID_Product WHERE DateC LIKE ? GROUP BY Orders.ID_Order";
            sqlite3_stmt *statement;
            // Prepare the SQL query
            if (sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
                // Bind the parameters
                 String selectedDatePattern = selectedYearStr + "-" + selectedMonthStr + "-%";
                  sqlite3_bind_text(statement, 1, selectedDatePattern.c_str(), -1, SQLITE_TRANSIENT);
                  
                String orderDetails;
                // Execute the SQL query and fetch the results
                while (sqlite3_step(statement) == SQLITE_ROW) {
                    // Extract order details from the current row
                    int orderID = sqlite3_column_int(statement, 0);
                    String orderDate = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
                    String status = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
                    String name = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 3)));
                    String surname = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
                    String email = String(reinterpret_cast<const char*>(sqlite3_column_text(statement, 5)));
                    double totalPrice = sqlite3_column_double(statement, 6);

                    // Construct a table row for the current order
                    orderDetails += "<tr><td>" + String(orderID) + "</td><td>" + name + " " + surname + "</td><td>" + email + "</td><td>" + orderDate + "</td><td>" + String(totalPrice) + "€</td><td>" + status + "</td></tr>";
                }

                // Insert order details into the content
                content.replace("<!-- Order details will be dynamically inserted here -->", orderDetails);

                // Send the HTTP response with the admin page content
                server.send(200, "text/html", content);

                // Finalize the SQL statement
                sqlite3_finalize(statement);
            } else {
                // If the SQL query execution fails, send an error response
                server.send(500, "text/plain", "Internal Server Error");
            }
        } else {
            // If the client is not authorized (client ID is not 0), send a forbidden response
            server.send(403, "text/plain", "Forbidden");
        }
    } else {
        // If the request method is not GET, send a forbidden response
        server.send(403, "text/plain", "Forbidden");
    }
}



