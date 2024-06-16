#include <iostream>
#include <vector>

#include "./Modbus/Modbus.hpp"

using namespace std;


int main(){
    vector<Modbus*> modbuss(1);
    cout << "Modbuss create success\n";
    modbuss[0] = new Modbus("0.0.0.0", 5000);
    int transaction = 0;
    int isContinue = 1;
    
    while ( isContinue){
        for ( Modbus* modbus: modbuss){
            int functionCode;
            cout << "functionCode: ";
            cin >> functionCode;

            int startAddr;
            cout << "startAddr: ";
            cin >> startAddr;

            int len = 1;
            if ( functionCode <= 4 || functionCode > 6){
                cout << "len: ";
                cin >> len;
            }

            uint16_t* data = new uint16_t[len];
            if ( functionCode >= 5){
                for ( int i = 0; i< len; i++){
                    int hi, lo;
                    cout << "hi data " << i << ": ";
                    cin >> hi;

                    cout << "lo data " << i << ": ";
                    cin >> lo;

                    data[i] = (hi << 8) | (lo << 0);
                }
            }

            cout << modbus->modbus_request(
                    ++transaction,
                    startAddr,
                    len,
                    functionCode,
                    data
            ) << endl;
        }

        cout << "continue? ";
        cin >> isContinue;
    }
    return 0;
}
