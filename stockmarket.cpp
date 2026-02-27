//STOCK MARKET TRADING PLATFORM 

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <limits>
#include <thread>
#include <chrono>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

//================COLOR CODES=================
#ifdef _WIN32
class ConsoleColor {
    private:
    HANDLE hConsole;
    int currentColor;

    public:
    ConsoleColor(){
        hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
        currentColor=7;     //default color
    }

    void setColor(int color){
        currentColor=color;
        SetConsoleTextAttribute(hConsole,color);
    }

    void reset(){
        currentColor=7;
        SetConsoleTextAttribute(hConsole,7);
    }
    int getColor() const { return currentColor; }

};

ConsoleColor consoleColor;

#define RED consoleColor.setColor(12)
#define GREEN consoleColor.setColor(10)
#define YELLOW consoleColor.setColor(14)
#define CYAN consoleColor.setColor(11)
#define MAGENTA consoleColor.setColor(13)
#define WHITE consoleColor.setColor(15)
#define BLUE consoleColor.setColor(9)
#define GRAY consoleColor.setColor(8)

#define RESET consoleColor.reset()
#define BOLD consoleColor.setColor(15)

//Helper functions for windows
ostream& operator<<(ostream& os, void(*f)(void)){
    f();
    return os;
}

#else
//ANSI Color codes for Linux/macOS
#define RED cout<<"\033[31m"
#define GREEN cout<<"\033[32m"
#define YELLOW cout<<"\033[33m"
#define CYAN cout<<"\033[36m"
#define MAGENTA cout<<"\033[35m"
#define BLUE cout<<"\033[34m"
#define WHITE cout<<"\033[57m"
#define GRAY cout<<"\033[90m"
#define BOLD cout<<"\033[1m"
#define RESET cout<<"\033[0m"
#endif


//-------------STOCK STRUCTURE---------
struct Stock{
    string symbol;      //variable for stock details
    string name;
    double price;
    double change;
    double changePercent;
    int volume;

    Stock(string s,string n,double p)
        : symbol(s),name(n),price(p),change(0.0),changePercent(0.0),volume(1000000){}   //constructor 

        void updatePrice(){
            double oldPrice=price;  //Step 1: Store Old Price
            double percentChange=((rand()%200)-100)/1000.0; // -10% to +10%
            price += price * percentChange;    //Step 3:Price changes by percentage.

            if(price<1.0)           //Step 4: Prevent Price Going Below 1
                price=1.0;

            change=price-oldPrice;  //Step 5: Calculate Price Change
            changePercent=(change/oldPrice)*100.0;      //Step 6: Calculate Percentage Change

            volume+=(rand()%50000)-25000;           //Randomly increase or decrease volume between:-25000 to +24999
            if(volume < 100000)         //Minimum trading volume is 100,000.
                volume=100000;
        }
};


//--------------TRADE STRUCTURE----------
struct Trade{
    string symbol;
    string type;
    int quantity;
    double price;
    double total;
    string time;
};

class Portfolio{
    private:
        double balance;     //Cash available in the account.
        double initialBalance;  //Initial balance for correct P/L
        map<string,int> holdings;   //How many shares you own for each stock symbol.
        vector<Trade> tradeHistory;     //List of all trades (BUY/SELL) done by the user.
    public:
        Portfolio(double startBalance =10000.0): balance(startBalance), initialBalance(startBalance){}

        //When you create a portfolio, it starts with default â‚¹/$ 10,000 unless you pass another value.


        double getBalance() const{return balance;}


        double getInitialBalance() const { return initialBalance; }
        map<string, int> getHoldings() const{return holdings;}
        vector<Trade> getHistory() const {return tradeHistory;}
        //give current balance, holdings, and trade history. 

        bool buyStock(const Stock& stock,int quantity){
            double cost=stock.price*quantity;       //Calculate total cost  (price=100, quantity=5 â†’ cost=500)

            if(cost>balance)        //Check if enough money
                return false;
            balance-=cost;          //Deduct money

            holdings[stock.symbol]+=quantity;       //Add shares to holdings
            
            Trade trade;    //Create a Trade record
            trade.symbol=stock.symbol;
            trade.type="BUY";
            trade.quantity=quantity;
            trade.price=stock.price;
            trade.total=cost;

            time_t now=time(nullptr);   //Store current time
        
#if defined(_MSC_VER)
            tm localTm;
            localtime_s(&localTm,&now);
            char buf[64];
            strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&localTm);
            trade.time=buf;
        
#else   
            tm* localTm=localtime(&now);
            char buf[64];
            strftime(buf,sizeof(buf), "%Y-%m-%d %H:%M:%S", localTm);
            trade.time=buf;
#endif
            tradeHistory.push_back(trade);  //Save trade in history
            return true;
        }
        bool sellStock(const Stock& stock,int quantity){
            auto it=holdings.find(stock.symbol);        //Find stock in holdings
            if(it==holdings.end()||it->second<quantity)     //Check enough quantity
                return false;

            double revenue=stock.price*quantity;    //Calculate revenue
            balance+=revenue;       //Add money to balance
            it->second-=quantity;   //Reduce holdings quantity

            if(it->second==0){      //If quantity becomes 0, remove entry
                holdings.erase(it);
            }

            Trade trade;        //Create SELL trade record
            trade.symbol=stock.symbol;
            trade.type="SELL";
            trade.quantity=quantity;
            trade.price=stock.price;
            trade.total=revenue;

            time_t now=time(nullptr);

#if defined(_MSC_VER)
            tm localTm;
            localtime_s(&localTm,&now);
            char buf[64];
            strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&localTm);
            trade.time=buf;
#else   
            tm* localTm=localtime(&now);
            char buf[64];
            strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",localTm);
            trade.time=buf;
#endif
            tradeHistory.push_back(trade);
            return true;
        }

        //Cash + value of all owned stocks
        double getPortfolioValue(const vector<Stock>& stocks) const{
            double total=balance;
            for(const auto& holding:holdings){
                for(const auto& stock:stocks){
                    if(stock.symbol==holding.first){
                        total+=holding.second*stock.price;
                        break;
                    }
                }
            }
            return total;
        } 
};

//--------------MARKET CLASS
class Market{
    private:
        vector<Stock> stocks;       // Container storing all available stocks in the market
    public:
        Market(){           //10 default companies are added
            stocks.push_back(Stock("AAPL","Apple Inc.",175.25));
            stocks.push_back(Stock("GOOGL","Google Inc.",138.75));
            stocks.push_back(Stock("MSFT","Microsoft Corp.",330.45));
            stocks.push_back(Stock("TSLA","Tesla Inc.",210.30));
            stocks.push_back(Stock("AMZN","Amazon Inc.",145.80));
            stocks.push_back(Stock("NVDA","NVIDIA Corp.",480.90));
            stocks.push_back(Stock("META","Meta Platforms",310.25));
            stocks.push_back(Stock("NFLX","Netflix Inc.",485.60));
            stocks.push_back(Stock("AMD","AMD Inc.",122.35));
            stocks.push_back(Stock("INTC","Intel Corp.",44.80));
        }
        void updatePrices(){    //Updates price and volume of all stocks.
            for(auto& stock : stocks){
                stock.updatePrice();    // Call Stock's update logic
            }
        }

        /* Displays real-time market quotes in formatted table.
         Applies dynamic color formatting based on:
         - Price range
         - Price change
         - Trading volume*/

        void displayMarket() const{
            CYAN;
            cout<<"\n===========================================================\n";
            cout<<"                    REAL - TIME MARKET QUOTES                 \n";
            cout<<"\n============================================================\n";
            RESET;

            CYAN;
            cout<<"\n+------------+-------------------+---------------+----------+-------------+---------------+\n";
            cout<<"|    SYMBOL  |       COMPANY     |   LAST PRICE  |   CHANGE  |   %CHANGE   |     VOLUME    |\n";
            cout<<"\n+------------+-------------------+---------------+----------+-------------+---------------+\n";
            RESET;

            // Loop through each stock and print its details
            for(const auto& stock : stocks){
                 // Print symbol and company name
                WHITE;
                cout<<" | "<<left<<setw(8)<<stock.symbol<<" | "<<setw(20)<<stock.name.substr(0,19);

                //Price with color based on value
                if(stock.price>300)
                    MAGENTA;                // High-value stock
                else if(stock.price>100)
                    CYAN;                   // Medium-value stock
                else 
                    YELLOW;                 // Lower-value stock
                
                cout<<" | $"<<setw(11) <<right<<fixed<<setprecision(2)<<stock.price;

                 // Apply color based on price movement
                if(stock.change>1.0)        // Positive movement
                    GREEN;
                else if(stock.change>0.1)
                    GREEN;
                else if(stock.change<-1.0)      // Negative movement
                    RED;
                else if(stock.change<-0.1)      // Neutral
                    RED;
                else
                    YELLOW;
                
                    // Print change and percentage change
                if(stock.change>0)
                {
                    cout<<"| +"<<setw(11)<< right << fixed << setprecision(2) <<stock.change;
                    cout<<"| +"<<setw(11)<< right << fixed << setprecision(2) <<stock.changePercent<<"%"; 
                }
                else if(stock.change <0){
                    cout<<"| "<<setw(12) << right << fixed << setprecision(2) <<stock.change;
                    cout<<"| "<<setw(12) <<right << fixed << setprecision(2) << stock.changePercent << "%";
                }
                else{
                    YELLOW;
                    cout<<"| "<<setw(12)<<right<<"0.00";
                    cout<<"| "<<setw(12)<<right<<"0.00%";
                }

                //VOLUME WITH COLOR BASED ON ACTIVITY
                WHITE;
                if(stock.volume >2000000)
                    MAGENTA;                        // Very high activity
                else if(stock.volume>1000000)
                    CYAN;                          // Moderate activity
                else
                    YELLOW;                       // Low activity

                cout<<"| "<<setw(9)<<right<<stock.volume<<" |\n";
                RESET;
            }
            CYAN;
            cout<<"\n+------------+-------------------+---------------+----------+-------------+---------------+";
            RESET;
        }
        Stock* findStock(const string& symbol){     //Searches for a stock by symbol.
            for(auto& stock:stocks){
                if(stock.symbol==symbol){
                    return &stock;
                }
            }// Returns pointer to stock if found, otherwise nullptr.
            return nullptr;
        }

        const vector<Stock>& getStocks() const{return stocks;}// Returns list of all stocks in the market.
};

//------------------------ENHANCED ANIMATION FUNCTIONS------------
void clearScreen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void sleepMilliseconds(int ms){
    #ifdef _WIN32
        Sleep(ms);
    #else  
        usleep(ms*1000);
    #endif
}

void showHashLoading(const string& message = "Processing"){
    cout<<"\n";
    MAGENTA;
    cout<<message<<" [";
    RESET;

    string hashChars="#";
    string pipeChars="|";
    string dashChars="-";
    string starChars="*";

    for(int i=0;i<50;i++){
        if(i%4==0){
            GREEN;
            cout<<"#";
        }
        else if(i%4==1){
            CYAN;
            cout<<"|";
        }
        else if(i%4==2){
            YELLOW;
            cout<<"-";
        }
        else{
            MAGENTA;
            cout<<"*";
        }
        cout.flush();
        sleepMilliseconds(30);
    }
    GREEN;
    cout<<"] DONE!\n";
    RESET;
    sleepMilliseconds(300);
}

void showRealTimeTicker(){
    BLUE;
    cout<<"\nMARKET TICKER:";
    RESET;

    vector<string> tickerSymbols={"AAPL","GOOGL","MSFT","TSLA","AMZN","NVDA","META","NFLX"};
    for(int i=0;i<12;i++){
        string symbol=tickerSymbols[rand() % tickerSymbols.size()];
        double price=100+(rand() % 1000);
        double change=(rand() % 20)-10;

        if(change>0){
            GREEN;
            cout<<symbol<<"$"<<fixed<<setprecision(2)<<price/10.0<<"UP"<<change/10.0<<"  ";
        }

        else{
            RED;
            cout<<symbol<<"$"<<fixed<<setprecision(2)<<price/10.0<<"DN"<<fabs(change)/10.0<<"  ";
        }
        cout.flush();
        sleepMilliseconds(80);
    }
    RESET;
    cout<<"\n";
}

void showPlatformBootAnimation(){
    clearScreen();

    CYAN;
    cout<<"\n#########################################################################################\n";
    cout<< "#                                TRADING TERMINAL v3.0                                   #\n";
    cout<< "#########################################################################################\n\n";
    RESET;

    vector<string> bootMessages={
        "Initializing trading engine...",
        "Connecting to market data feeds...",
        "Loading portfolio manager...",
        "Setting up risk management...",
        "Authenticating user session...",
        "Establishing secure connection..."
    };
    //const function

    for(const auto& msg : bootMessages){
        YELLOW;
        cout<<"["<<msg<<"]";
        RESET;
        cout<<"[";
        for(int i=0;i<25;i++){
            if(i%3==0){
                GREEN;
                cout<<"#";
            }
            else if(i%3==1){
                CYAN;
                cout<<"|";
            }
            else{
                BLUE;
                cout<<"-";
            }
            cout.flush();
            sleepMilliseconds(30+rand()%30);
        }
        GREEN;
        cout<<"] OK\n";
        RESET;
        sleepMilliseconds(200);
    }
    cout<<"\n";
    GREEN;
    cout<<"=================================================================================\n";
    cout<<"                   TRADING PLATFORM READY - MARKET DATA STREAMING LIVE           \n";
    cout<<"==================================================================================\n";
    RESET;
    sleepMilliseconds(1000);
}

void showOrderProcessingAnimation(const string& orderType,const string& symbol,int quantity){
    clearScreen();
    CYAN;
    cout<<"\n###############################################################################\n";
    cout<<"#                        ORDER EXECUTION IN PROGRESS                             #\n";
    cout<<"#################################################################################\n\n";
    RESET;

    WHITE;
    cout<<"Order Type: ";
    if(orderType=="BUY"){
        GREEN;
    }else{
        RED;
    }
    cout<<orderType;
    RESET;
    cout<<"\nSymbol: ";
    CYAN;
    cout<<symbol;
    RESET;
    cout<<"\nQuantity:";
    YELLOW;
    cout<<quantity;
    RESET;
    cout<<" shares\n\n";

    //Advanced processing animation
    vector<string> steps={
        "Validating order parameters...",
        "Checking market liquidity...",
        "Confirming price...",
        "Executing on exchange...",
        "Processing settlement...",
        "Updating portfolio..."
    };

    for(int step=0;step<steps.size();step++){
        WHITE;
        cout<<" "<<steps[step];

        //Dynamic progress indicator with hash symbols
        for(int i=0;i<3;i++){
            cout<<".";
            cout.flush();
            sleepMilliseconds(100);
        }
        //Random success/failure simulation
        if(rand()%10>1){    //80% success rate
            GREEN;
            cout<<" SUCCESS #\n";
        }
        else{
            RED;
            cout<<" RETRYING\n";
            sleepMilliseconds(300);
            GREEN;
            cout<<"           SUCCESS #\n";
        }
        RESET;
        sleepMilliseconds(200);
    }
    //Final confirmation with hash bar
    cout<< "\n";
    GREEN;
    for(int i=0;i<60;i++){
        if(i%2==0) cout<<"#";
        else cout<<"|";
        cout.flush();
        sleepMilliseconds(15);
    }
    cout<<"\n\nORDER EXECUTED SUCCESSFULLY\n";
    RESET;
    sleepMilliseconds(1500);
}

void showLiveMarketChart(){
    BLUE;
    cout<<"\nLIVE MARKET CHART:\n";
    RESET;

    int width=60;
    int height=10;

    //create a simple bar chart simulation with hash symbols
    for(int row=height;row>=0;row--){
        cout<<"|";
        for(int col=0;col<width;col++){
            int value=(sin(col * 0.3 + rand() % 100 * 0.01) * 0.5 + 0.5) * height;

            if(col%4==0){
                //bar chart
                if(value>row){
                    if(rand()%2==0){
                        GREEN;
                        cout<<"#";
                    }
                    else{
                        RED;
                        cout<<"|";
                    }
                }else if(fabs(value-row)<0.5){
                    YELLOW;
                    cout<<"-";
                }else{
                    cout<<" ";
                }
            }
            else{
                //line chart
                if(fabs(value-row)<0.3){
                    if(rand()%3==0){
                        CYAN;
                        cout<<".";
                    }
                    else{
                        cout<<" ";
                    }
                }
                else{
                    cout<<" ";
                }
            }
        }
        cout<<"|\n";
        sleepMilliseconds(20);
    }
    BLUE;
    cout<<"+";
    for(int i=0;i<width;i++) cout<<"-";
    cout<<"+\n";

    //time labels
    cout<<" 9:30    10:30   11:30   12:30   13:30   14:30   15:30   16:00\n";
    RESET;
}

//===========================USER INTERFACE FUNCTIONS===============
void displayMainMenu(Portfolio& portfolio,Market& market){
    double portfolioValue = portfolio.getPortfolioValue(market.getStocks());
    double profitLoss=portfolioValue-portfolio.getInitialBalance();

    //header with live stats
    BLUE;
    cout<<"\n=========================================================================\n";
    cout<<"                          TRADING PLATFORM                           \n";
    cout<<"============================================================================\n";
    YELLOW;
    cout<<" Cash: ";
    GREEN;
    cout<<"$" <<left <<setw(10)<<fixed<<setprecision(2)<<portfolio.getBalance();
    YELLOW;
    cout<<"| Portfolio: ";
    if(profitLoss >=0){
        GREEN;
        cout<<"$"<<left<<setw(10)<<portfolioValue;
        cout<<" (+$"<<profitLoss<<")";
    }
    else{
        RED;
        cout<<"$"<<left<<setw(10)<<portfolioValue;
        cout<<" (-$"<<fabs(profitLoss)<<")";
    }
    YELLOW;
    cout<<"| Session: ";
    CYAN;
    cout<<"LIVE";
    cout<<"\n";

    BLUE;
    cout<<"================================================================================\n";
    RESET;
    
    //main menu 
    cout<<"\n";
    MAGENTA;
    cout<<"---------------------------------------------------------------------------------\n";
    cout<<"-                              MAIN MENU                                        -\n";
    cout<<"--------------------------------------------------------------------------------\n\n";
    RESET;
    vector<pair<string,string>> menuOptions ={
        {"1","View Live Market"},
        {"2","My Portfolio"},
        {"3","Buy Stocks"},
        {"4","Sell Stocks"},
        {"5","Trade History"},
        {"6","Market Analysis"},
        {"7","Exit"}
    };
    for(int i=0;i<menuOptions.size();i+=2){
        for(int j=0;j<2 && i+j <menuOptions.size();j++){
            const auto& option = menuOptions[i+j];
            CYAN;
            cout<<" ["<<option.first<<"]";
            WHITE;
            cout<<left<<setw(30)<<option.second;
            RESET;
        }
        cout<<"\n";
    }
    cout<<"\n";
    YELLOW;
    cout<<"------------------------------------------------------------------------------\n";
    RESET;

    //show market status
    auto stocks=market.getStocks();
    int gainers=0,losers=0;
    for(const auto& stock : stocks){
        if(stock.change>0) gainers++;
        else if(stock.change<0) losers++;
    }
    WHITE;
    cout<<"Market Status: ";
    if(gainers>losers*1.5){
        GREEN;
        cout<<"BULLISH";
        for(int i=0;i<3;i++){
            cout<<"^^^ ";
            sleepMilliseconds(100);
        }
    }else if(losers > gainers*1.5){
        RED;
        cout<<"BEARISH ";
        for(int i=0;i<3;i++){
            cout<<"âŒ„âŒ„âŒ„";
            sleepMilliseconds(100);
        }
    } else{
        YELLOW;
        cout<<"NEUTRAL ";
        for(int i=0;i<3;i++){
            cout<<"--- ";
            sleepMilliseconds(100);
        }
    }
    RESET;

    cout<<" (Gainer: ";
    GREEN;
    cout<<gainers;
    RESET;
    cout<<" | Losers: ";
    RED;
    cout<<losers;
    RESET;
    cout<<")\n\n";
}

void displayPortfolio(Portfolio& portfolio,Market& market){
    CYAN;
    cout<<"============================================================================\n";
    cout<<"                        MY PORTFOLIO                                        \n";
    cout<<"============================================================================\n";
    RESET;

    double portfolioValue = portfolio.getPortfolioValue(market.getStocks());
    double profitLoss=portfolioValue-portfolio.getInitialBalance();

    WHITE;
    cout<<"\n+--------------------------------------------------------------------------------+\n";
    cout<<"| Account Summary                                                                   |\n";
    cout<<"+-----------------------------------------------------------------------------------+\n";
    RESET;

    YELLOW;
    cout<<"| Cash Balance: $"<<setw(38)<<right<<fixed<<setprecision(2)<<portfolio.getBalance()<<" |\n";
    cout<<"| Portfolio Value: $"<<setw(38)<<right<<portfolioValue<<" |\n";

    cout<<"| Profit/Loss:   ";
    if(profitLoss >=0.0){
        GREEN;
        cout<<"+$"<<setw(37)<<right<<profitLoss<<" |\n";
        YELLOW;
        cout<<"| Change:        "<<setw(37)<<right<<fixed<<setprecision(2)<<(profitLoss/10000.0)*100.0<<"% |\n";
    }
    else{
        RED;
        cout<<"-$"<<setw(37)<<right<<fabs(profitLoss)<<" |\n";
        YELLOW;
        cout<<"| Change:        "<<setw(38)<<right << fixed << setprecision(2)<<(profitLoss/10000.0)*100.0<<"% |\n";
    }
    WHITE;
    cout<<"+------------------------------------------------------------------+\n";
    RESET;

    cout<<"\n";
    CYAN;
    cout<<"Your Holdings:\n";
    cout<<"+----------------+------------------+--------------------+-------------------+\n";
    cout<<"|    Symbol      |       Shares     |   Current Price    |   Total Value      |\n";
    cout<<"+-----------------------------------------------------------------------------+\n";
    RESET; 

    auto holdings=portfolio.getHoldings();
    auto stocks=market.getStocks();
    if(holdings.empty()){
        YELLOW;
        cout<<"|"<<setw(58)<<"No Stocks Owned.Visit the market to buy stocks!"<<"|\n";
    }
    else{
        for(const auto& holding :holdings){
            for(const auto& stock:stocks){
                if(stock.symbol == holding.first){
                    double value=holding.second*stock.price;
                    WHITE;
                    cout<<"|"<<left<<setw(8)<<holding.first<<"|"<<setw(8)<<holding.second<<"| $"<<setw(14)<<right<<fixed<<setprecision(2)<<stock.price<<"| $"<<setw(13)<<right<<value<<" |\n";
                    break;
                }
            }
        }
    }
    CYAN;
    cout<<"+----------------+------------------+--------------------+-------------------+\n";
    RESET;
}

//--------------------------ENHANCED BUY FUNCTION------------------
void enhancedBuyStock(Portfolio& portfolio,Market& market){
    market.displayMarket();

    GREEN;
    cout<<"\n###########################################################################\n";
    cout<<"#                              BUY ORDER                                     #\n";
    cout<<"##############################################################################\n\n";
    RESET;
    cout<<"Enter stock symbol: ";
    string symbol;
    cin>>symbol;
    transform(symbol.begin(),symbol.end(),symbol.begin(), ::toupper);

    Stock* stock=market.findStock(symbol);
    if(!stock){
        RED;
        cout<<"\nERROR: Symbol '"<<symbol<<"' not found!\n";
        RESET;
        sleepMilliseconds(1500);
        return;
    }
    //show detailed stock info
    CYAN;
    cout<<"\n+----------------------------------------------------------+\n";
    cout<<" | "<<left<<setw(68)<<stock -> name<<" |\n";
    cout<<"+------------------------------------------------------------+\n";
    RESET;

    WHITE;
    cout<<" | Symbol:       ";
    CYAN;
    cout<<stock->symbol;
    RESET;
    cout<<"\n| Current Price:   ";
    GREEN;
    cout<<"$"<<fixed<<setprecision(2)<<stock->price;
    RESET;
    cout<<"\n| Daily Change:    ";
    if(stock->change >0){
        GREEN;
        cout<<"+$"<<stock->change<<" (+"<<stock->changePercent<<"%)";
    }
    else{
        RED;
        cout<<"-$"<<fabs(stock->change)<<" ("<<stock->changePercent<<"%)";
    }
    RESET;
    cout<<"\n| Volume:      ";
    YELLOW;
    cout<<stock->volume;
    RESET;
    cout<<" shares\n| Your Balance:     ";
    GREEN;
    cout<<"$"<<portfolio.getBalance();
    RESET;

    int maxShares=static_cast<int>(portfolio.getBalance() / stock->price);
    cout<<"\n| Max Purchases:       ";
    YELLOW;
    cout<<maxShares;
    RESET;
    cout<<" shares\n";

    CYAN;
    cout<<"+------------------------------------------------------------+\n\n";
    RESET;

    cout<<"Enter quantity: ";
    int quantity;
    cin>>quantity;

    if(quantity <=0){
        RED;
        cout<<"\nInvalid Quantity!\n";
        RESET;
    }
    else if(quantity>maxShares){
        RED;
        cout<<"\nInsufficient funds!\n";
        cout<<"     Required: $"<<fixed<<setprecision(2)<<quantity*stock->price<<"\n";
        cout<<" Available: $"<<portfolio.getBalance()<<"\n";
        RESET;
    }
    else{
        showOrderProcessingAnimation("BUY",symbol,quantity);

        if(portfolio.buyStock(*stock,quantity)){
            //Success animation with hash symbols
            for(int i=0;i<3;i++){
                GREEN;
                cout<<"$$$";
                cout.flush();
                sleepMilliseconds(300);
            }
            GREEN;
            cout<<"\n\nORDER CONFIRMED!\n";
            cout<<"     Bought"<<quantity<<"shares of"<<symbol<<" at $"<<stock->price<<"\n";
            cout<<"     Total Cost: $"<<fixed<<setprecision(2)<<quantity*stock->price<<"\n";
            RESET;
        }
    }
    cout<<" \nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

//-------------------------------ENHANCED SELL FUNCTION-------------
void enhancedSellStock(Portfolio& portfolio,Market& market){
    auto holdings=portfolio.getHoldings();
    if(holdings.empty()){
        YELLOW;
        cout<<"\nYou don't own any stocks to sell!\n";
        RESET;
        cout<<"\nPress Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }
    YELLOW;
    cout<<"\n+------------------------------------------------------------+\n";
    cout<<"|                        SELL STOCKS                            |\n";
    cout<<"+------------------------------------------------------------+\n";
    RESET;

    cout<<"\n";
    CYAN;
    cout<<"Your Current Holdings:\n";
    cout<<"+----------------+------------------+--------------------+-------------------+\n";
    cout<<"|    Symbol      |       Shares     |   Current Price    |   Total Value      |\n";
    cout<<"+-----------------------------------------------------------------------------+\n";
    RESET;

    auto stocks=market.getStocks();
    for(const auto& holding : holdings){
        for(const auto& stock :stocks){
            if(stock.symbol==holding.first){
                double value=holding.second*stock.price;
                WHITE;
                cout<<"| "<<left << setw(8)<<holding.first<<"| "<< setw(8)<<holding.second<<"| $"<< setw(14)<< right << fixed << setprecision(2)<<stock.price<<"| $"<< setw(13)<< right<< value<<" |\n";
                break;
            }
        }
    }
    CYAN;
    cout<<"+----------------+------------------+--------------------+-------------------+\n";
    RESET;
    cout<<"Enter stock symbol to sell: ";
    string symbol;
    cin>>symbol;
    transform(symbol.begin(),symbol.end(),symbol.begin(),::toupper);

    Stock* stock=market.findStock(symbol);
    if(!stock){
        RED;
        cout<<"\nStock symbol not found!\n";
        RESET;
        cout<<"\nPress Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }
    int ownedShares=holdings[symbol];
    cout<<"You own: ";
    CYAN;
    cout<<ownedShares;
    RESET;
    cout<<" shares\n";
    cout<<"Current price: ";
    GREEN;
    cout<<"$" <<fixed <<setprecision(2)<<stock->price;
    RESET;
    cout<<"\n";
    cout<<"Enter quantity to sell: ";
    int quantity;
    cin>>quantity;

    if(quantity<=0 || quantity>ownedShares){
        RED;
        cout<<"\nInvalid quantity!\n";
        RESET;
    }
    else if(portfolio.sellStock(*stock,quantity)){
        showOrderProcessingAnimation("SELL",symbol,quantity);
        GREEN;
        cout<<"\nSUCCESS! Sold"<<quantity<<" shares of "<<symbol<<"\n";
        cout<<"Total received: $"<< fixed <<setprecision(2)<< stock->price*quantity<<"\n";
        RESET;
    }
    else{
        RED;
        cout<<"\nSale Failed!\n";
        RESET;
    }

    cout<<"\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}


//---------------------------MAIN FUNCTION-----------------
int main(){
    srand(static_cast<unsigned int>(time(nullptr)));

    Market market;
    Portfolio portfolio(10000.0);
    //show boot animation
    showPlatformBootAnimation();

    //initial market update
    market.updatePrices();
    
    while(true){
        market.updatePrices();
        clearScreen();
        //show live ticker at top
        showRealTimeTicker();

        //display enhanced main menu
        displayMainMenu(portfolio,market);


        CYAN;
        cout<<"\nSelect option(1-7): ";
        RESET;
        int choice;
        if(!(cin>>choice)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            RED;
            cout<<"\nInvalid input! Please enter a number 1-7.\n";
            RESET;
            sleepMilliseconds(1500);
            continue;
        }
        switch (choice)
        {
        case 1:{
            clearScreen();
            market.displayMarket();
            showLiveMarketChart();
            cout<<"\nPress Enter to continue...";
            cin.ignore();
            cin.get();
            break;
        }
        case 2: {
            clearScreen();
            displayPortfolio(portfolio,market);
            cout<<"\nPress Enter to continue...";
            cin.ignore();
            cin.get();
            break;
        }
        case 3:{
            clearScreen();
            enhancedBuyStock(portfolio,market);
            break;
        }
        case 4:{
            clearScreen();
            enhancedSellStock(portfolio,market);
            break;
        }
        case 5:{
            clearScreen();
            auto history=portfolio.getHistory();

            CYAN;
            cout<<"\n====================================================================\n";
            cout<<"                               TRADE HISTORY                          \n";
            cout<<"\n====================================================================\n";
            RESET;

            if(history.empty()){
                YELLOW;
                cout<<"\nNo trades yet. Start trading to see history here!\n";
                RESET;
            }
            else{
                cout<<"\n";
                CYAN;
                cout<<"+------------+-------------+----------+--------------+-------------+---------------+\n";
                cout<<"|   TYPE     |    SYMBOL   |   QTY    |     PRICE    |     TOTAL   |      TIME      |\n";
                cout<<"+------------+-------------+----------+--------------+-------------+---------------+\n";
                RESET;

                for(const auto& trade : history){
                    if(trade.type=="BUY"){
                        GREEN;
                    }
                    else{
                        RED;
                    }
                    cout<<"| "<<left << setw(7)<< trade.type << "| " << setw(8) << trade.symbol <<"| "<< setw(6) << trade.quantity <<"| $" << setw(9) << fixed << setprecision(2) << trade.price << "| $"<< setw(12) << fixed << setprecision(2) << trade.total << "| "<< trade.time << " |\n";
                    RESET;
                }
                CYAN;
                cout<<"+------------+-------------+----------+--------------+-------------+---------------+\n";
                RESET;
            }
            cout<<"\nPress Enter to continue...";
            cin.ignore();
            cin.get();
            break;
        }
        case 6: {
            clearScreen();
            auto stocks=market.getStocks();
            CYAN;
            cout<<"\n========================================================================================\n";
            cout<<"                                    MARKET ANALYSIS                                       \n";
            cout<<"\n========================================================================================\n";
            RESET;
            int gainers=0,losers=0;
            double totalChange=0.0;
            for(const auto& stock:stocks){
                totalChange += stock.changePercent;
                if(stock.change >0.0) ++gainers;
                else if(stock.change<0.0) ++losers;
            }
            double avgChange=stocks.empty() ? 0.0:totalChange /static_cast<double>(stocks.size());
            cout<<"\n";
            YELLOW;
            cout<<"Market Overview:\n";
            cout<<"+--------------------+----------------------+\n";
            cout<<"| Total Stocks: "<<setw(10) << left << stocks.size() <<" | Gainers: "<< setw(12) << gainers << " |\n";
            cout<<"+--------------------+----------------------+\n";
            cout<<"| Losers: "<<setw(15) << left << losers << " | Avg Change: "<< setw(9) << fixed << setprecision(2) << avgChange << "% |\n";
            cout<<"+--------------------+----------------------+\n";
            RESET;

            //TOP GAINERS
            cout<<"\n";
            GREEN;
            cout<<"TOP GAINERS: \n";
            vector<Stock> sorted=stocks;
            sort(sorted.begin(),sorted.end(),
                [](const Stock& a,const Stock& b){return a.changePercent > b.changePercent; });

            for(int i=0;i<min(3,(int)sorted.size()); ++i){
                if(sorted[i].changePercent>0.0){
                    cout<<" "<<sorted[i].symbol<< ": +" <<fixed << setprecision(2) << sorted[i].changePercent<<"%\n";
                }
            }

            //Top Losers
            cout<<"\n";
            RED;
            cout<<"TOP LOSERS: \n";
            sort(sorted.begin(),sorted.end(),
            [](const Stock& a,const Stock& b){return a.changePercent<b.changePercent;});

            for(int i=0;i<min(3,(int)sorted.size());++i){
                if(sorted[i].changePercent<0.0){
                    cout<<" "<<sorted[i].symbol<<": "<<fixed << setprecision(20)<< sorted[i].changePercent << "%\n";
                }
            }
            cout<<"\n";
            CYAN;
            cout<<"Market Sentiment: ";
            if(avgChange >0.5){
                GREEN;
                cout<<"BULLISH^^^\n";
            }
            else if(avgChange< -0.5){
                RED;
                cout<<"BEARSIH âŒ„âŒ„âŒ„\n";
            }
            else{
                YELLOW;
                cout<<"NEUTRAL ---\n";
            }
            RESET;
            cout<<"\nPress Enter to continue...";
            cin.ignore();
            cin.get();
            break;
        }
        case 7:{
            clearScreen();
            showOrderProcessingAnimation("LOGOUT","SYSTEM",0);
            CYAN;
            cout<<"\n==========================================================================\n";
            cout<<"=               TRADING SESSION ENDED - THANK YOU!                         =\n";
            cout<<"\n==========================================================================\n";
            RESET;

            double finalValue=portfolio.getPortfolioValue(market.getStocks());
            cout<<"Final Portfolio Value: $"<<finalValue<< "\n";
            cout<<" Total Profit/Loss: ";
            if(finalValue >= portfolio.getInitialBalance()){
                GREEN;
                cout<<"$" << fixed << setprecision(2) << finalValue - portfolio.getInitialBalance();   
            }
            else{
                RED;
                cout<<"$" << fixed << setprecision(2) << finalValue- portfolio.getInitialBalance();
            }
            RESET;
            cout<<"\n\n";

            //final hash animation
            GREEN;
            cout<<"Closing system";
            showHashLoading("Closing system");

            cout<<"\nPress Enter to exit...";
            cin.ignore();
            cin.get();
            return 0;
        }

        default:{
            RED;
            cout<<"\nInvalid choice! Please select 1-7.\n";
            RESET;
            sleepMilliseconds(1500);
        }
        }
    }
    system("pause");
    return 0;
}
