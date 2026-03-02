#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>

using namespace std;

struct Stock {
    string symbol;
    string name;
    double price;
    double change = 0.0;
    double changePercent = 0.0;
    int volume = 1000000;

    Stock(string s, string n, double p) : symbol(s), name(n), price(p) {}

    void updatePrice() {
        double oldPrice = price;
        double percentChange = ((rand() % 200) - 100) / 1000.0; // -10% to +10%
        price += price * percentChange;
        if (price < 1.0) price = 1.0;

        change = price - oldPrice;
        changePercent = (change / oldPrice) * 100.0;

        volume += (rand() % 50000) - 25000;
        if (volume < 100000) volume = 100000;
    }
};

class Market {
public:
    vector<Stock> stocks;
    Market() {
        stocks.push_back(Stock("AAPL","Apple Inc.",175.25));
        stocks.push_back(Stock("GOOGL","Google Inc.",138.75));
        stocks.push_back(Stock("MSFT","Microsoft Corp.",330.45));
        stocks.push_back(Stock("TSLA","Tesla Inc.",210.30));
        stocks.push_back(Stock("AMZN","Amazon Inc.",145.80));
        stocks.push_back(Stock("NVDA","NVIDIA Corp.",480.90));
        stocks.push_back(Stock("META","Meta Platforms",310.25));
        stocks.push_back(Stock("NFLX","Netflix Inc.",485.60));
    }

    void update() {
        for (auto &s : stocks) s.updatePrice();
    }
};

static string readFile(const string& path) {
    ifstream f(path, ios::in | ios::binary);
    if (!f) return "";
    stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main() {
    srand((unsigned)time(nullptr));

    crow::SimpleApp app;
    Market market;

    // Serve HTML
    CROW_ROUTE(app, "/")([&]() {
        string html = readFile("index.html");
        if (html.empty()) return crow::response(404, "index.html not found in project folder.");
        crow::response res;
        res.code = 200;
        res.set_header("Content-Type", "text/html; charset=utf-8");
        res.body = html;
        return res;
    });

    // Market JSON API
    CROW_ROUTE(app, "/market")([&]() {
        market.update();
        crow::json::wvalue out = crow::json::wvalue::list();
        int i = 0;
        for (auto &s : market.stocks) {
            out[i]["symbol"] = s.symbol;
            out[i]["name"] = s.name;
            out[i]["price"] = s.price;
            out[i]["change"] = s.change;
            out[i]["changePercent"] = s.changePercent;
            out[i]["volume"] = s.volume;
            i++;
        }
        return out;
    });

    app.port(18080).multithreaded().run();
}