#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <map>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
using namespace std;

// Helper function for to_string since older compilers don't have it
string to_string(int value) {
    ostringstream oss;
    oss << value;
    return oss.str();
}

//----------------------STRUCT-------------------
struct paymentDetails {
    int paymentid;
    string paymentmethod;
    double amount;
    string transactiontime;
    string status;//"pending","complete","failed"
    string authorizationcode;
};

//------------------------helper-------------------
string getCurrentTime() {
    time_t now = time(0);
    char*dt = ctime(&now);
    string t(dt);
    if (!t.empty()) t.erase(t.size() - 1);
    return t;
}
//---------------------------CLASS:paymentprocessor--------------
class PaymentProcessor {
    private:
    static int nextpaymentid;
    paymentDetails*currentPayment;
    vector<paymentDetails*>transactionHistory;
    map<string, double>paymentstarts;
    string supportedmethod[4];

    void logError(const string& msg){
        ofstream err("payment_errors.log", ios::app);
        err<< "[" <<getCurrentTime() <<"]"<< msg << endl;
        err.close();
    }
void savetransactionToFile(const paymentDetails& p, const string& filename) {
    ofstream out(filename.c_str(), ios::app);
    if(out.is_open()){
        out << "PAY-" <<p.paymentid <<","<<p.paymentmethod <<","
        <<fixed<<setprecision(2) <<p.amount <<"," <<p.transactiontime << "," <<p.status <<"," << p.authorizationcode << endl;
        out.close();
    }
}
public:
PaymentProcessor(){
    nextpaymentid = 1001;
    currentPayment = NULL;
}
~PaymentProcessor(){
    if(currentPayment != NULL){
        delete currentPayment;
        currentPayment = NULL;
    }
    for (size_t i = 0; i < transactionHistory.size(); ++i) delete transactionHistory[i];
}

//..........................CASH PAYMENT....................
bool processCashPayment(double amount, double tendered){
    try{
        if(amount<=0)
        throw invalid_argument("invalid amount.");
        if(tendered < amount)
        throw runtime_error ("insufficient cash tendered");
        currentPayment = new paymentDetails;
        currentPayment->paymentid =nextpaymentid++;
        currentPayment->paymentmethod = "cash";
        currentPayment->amount = amount;
        currentPayment->transactiontime =
        getCurrentTime();
        currentPayment->authorizationcode ="CASH-"
        + to_string(rand() % 10000);
        currentPayment->status = "completed";

        double change = tendered - amount;
        cout << "\n===CASH PAYMENT===\n";
        cout <<"amount due: $"<<amount <<endl;
        cout <<"cash tendered: $" <<tendered<<endl;
        cout<<"change due: $" <<change <<endl;
        cout <<"payment status: APPROVED\n";
        cout <<"Transaction ID:PAY-"<< currentPayment->paymentid <<endl;

        savetransactionToFile(*currentPayment,"transactions.txt");
        paymentstarts["cash"] += amount;
        transactionHistory.push_back(currentPayment);
        currentPayment = NULL;
        return true;
    }catch (exception& e){
        logError(e.what());
        cout << "Cash Payment Failed: "<< e.what() <<endl;
        return false;
    }
}
//..................CARD VALIDATION..................
bool validateCard(string CardNumber,string expiry,string cvv){
    if(CardNumber.length() != 16) return false;
    if(expiry.size() != 5 || expiry[2] != '/')
    return false;
    if(cvv.size() < 3 || cvv.size() > 4)
    return false;
    return true;
}
//....................CARD PAYMENT............................
bool processCardPayment(double amount, string cardNumber,string expiry, string cvv,string cardtype)
{
    try{
        if(amount <= 0)
        throw invalid_argument("invalid amount. ");
        if(!validateCard(cardNumber,expiry,cvv))
        throw runtime_error("invalid card details.");

        currentPayment =new paymentDetails;
        currentPayment->paymentid = nextpaymentid++;
        currentPayment->paymentmethod =cardtype +"card";
        currentPayment->amount = amount;
        currentPayment->transactiontime =
        getCurrentTime();
        currentPayment->authorizationcode ="AUTH-"+to_string(rand() % 99999);
        bool declined = (rand() % 5 == 0);
        if(!declined){
          currentPayment->status = "completed" ;
          cout <<"\n=== " << cardtype <<" CARD PAYMENT===\n";
          cout <<"amount: $" <<amount <<endl;
          cout <<"Authorization Code: "<<currentPayment->authorizationcode <<endl;
          cout <<"Status: COMPLETED\n";
          savetransactionToFile(*currentPayment,"transactions.txt");
          paymentstarts[cardtype + "card"] += amount;
        } else {
          currentPayment->status = "failed" ;
          cout <<"\n=== " << cardtype <<" CARD PAYMENT===\n";
          cout <<"amount: $" <<amount <<endl;
          cout <<"Authorization Code: "<<currentPayment->authorizationcode <<endl;
          cout <<"Status: FAILED\n";
          savetransactionToFile(*currentPayment,"transactions.txt");
        }
        transactionHistory.push_back(currentPayment);
           currentPayment = NULL;
           return!declined;
    }catch(exception& e){
        logError(e.what());
        cout << "Card Payment Failed: " << e.what() << endl;
        return false;
    }
}
 
//..................MOBILE PAYMENT................................
bool processMobilePayment(double amount,string provider){
    try{
        if(amount <=0)
         throw invalid_argument("invalid amount.");
         bool timeout = rand() % 10 == 0;

         currentPayment = new paymentDetails;
         currentPayment->paymentid = nextpaymentid++;
         currentPayment->paymentmethod = "mobile payment";
         currentPayment->amount = amount;
         currentPayment->transactiontime = getCurrentTime();
         currentPayment->authorizationcode = "MOBILE-" + to_string(rand() % 10000);

         if(!timeout){
             currentPayment->status = "completed";
             cout << "\n===MOBILE PAYMENT===\n";
             cout << "amount: $" << amount << endl;
             cout << "provider: " << provider << endl;
             cout << "Authorization Code: " << currentPayment->authorizationcode << endl;
             cout << "Status: COMPLETED\n";
             savetransactionToFile(*currentPayment, "transactions.txt");
             paymentstarts["mobile payment"] += amount;
         } else {
             currentPayment->status = "failed";
             cout << "\n===MOBILE PAYMENT===\n";
             cout << "amount: $" << amount << endl;
             cout << "provider: " << provider << endl;
             cout << "Status: FAILED (Timeout)\n";
             savetransactionToFile(*currentPayment, "transactions.txt");
             logError("Mobile payment timeout for provider: " + provider);
         }
         transactionHistory.push_back(currentPayment);
         currentPayment = NULL;
         return !timeout;
    }catch(exception& e){
        logError(e.what());
        cout << "Mobile Payment Failed: " << e.what() << endl;
        return false;
    }
}
};

// Static member definition after the class
int PaymentProcessor::nextpaymentid = 1001;

int main() {
    PaymentProcessor processor;

    // Basic successful tests
    std::cout << "=== BASIC TESTS ===" << std::endl;
    std::cout << "Testing Cash Payment:" << std::endl;
    processor.processCashPayment(50.0, 60.0);

    std::cout << "\nTesting Card Payment:" << std::endl;
    processor.processCardPayment(100.0, "1234567890123456", "12/25", "123", "Credit");

    std::cout << "\nTesting Mobile Payment:" << std::endl;
    processor.processMobilePayment(25.0, "ProviderX");

    // Edge cases
    std::cout << "\n=== EDGE CASE TESTS ===" << std::endl;
    std::cout << "Testing Invalid Cash Amount (negative):" << std::endl;
    processor.processCashPayment(-10.0, 20.0);

    std::cout << "\nTesting Insufficient Cash:" << std::endl;
    processor.processCashPayment(50.0, 30.0);

    std::cout << "\nTesting Invalid Card (wrong length):" << std::endl;
    processor.processCardPayment(100.0, "123456789012345", "12/25", "123", "Credit");

    std::cout << "\nTesting Invalid Card (bad expiry):" << std::endl;
    processor.processCardPayment(100.0, "1234567890123456", "1225", "123", "Credit");

    std::cout << "\nTesting Invalid Mobile Amount (zero):" << std::endl;
    processor.processMobilePayment(0.0, "ProviderY");

    // Multiple transactions
    std::cout << "\n=== MULTIPLE TRANSACTIONS ===" << std::endl;
    for(int i = 0; i < 3; ++i) {
        std::cout << "\nTransaction " << (i+1) << " - Cash:" << std::endl;
        processor.processCashPayment(20.0 + i*10, 50.0);
    }

    std::cout << "\n=== SIMULATING RANDOM FAILURES (Run multiple times if needed) ===" << std::endl;
    for(int i = 0; i < 5; ++i) {
        std::cout << "\nRandom Card Payment " << (i+1) << ":" << std::endl;
        processor.processCardPayment(50.0, "1234567890123456", "12/25", "123", "Debit");
    }

    for(int i = 0; i < 5; ++i) {
        std::cout << "\nRandom Mobile Payment " << (i+1) << ":" << std::endl;
        processor.processMobilePayment(30.0, "ProviderZ");
    }

    return 0;
}
