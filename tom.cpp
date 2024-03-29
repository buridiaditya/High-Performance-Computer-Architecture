#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#define RS_NO 5
#define REG_NO 8 
#define MAX_INST_QUEUE 10

using namespace std;

typedef struct instruction{
    int opcode;
    int destination;
    int op1;
    int op2;
} instruction; // Instruction

typedef struct rs{
    int busy;
    int opcode;
    int vj;
    int vk;
    int qj;
    int qk;
    int disp;
} rs; // Reservation Station

void printState(vector<instruction>& instruction_queue, vector<rs>& RSs, vector<int>& RAT, vector<int>& RF){

    // Printing RSs
    cout << "    Busy  Op  Vj  Vk  Qj  Qk  Disp\n";
    for ( int i = 0; i < RS_NO; i++){
        cout << "RS"<< i << "   "<< RSs[i].busy << "   " << RSs[i].opcode << "   " << RSs[i].vj << "   " << RSs[i].vk << "   " << RSs[i].qj << "   " << RSs[i].qk << "   " << RSs[i].disp << endl;        
    }
    cout << "------------------------------------------\n";
    // Printing RF, RAT
    cout << "   RF  RAT\n";
    for ( int i = 0; i < REG_NO; i++){
        cout << i << ": " << RF[i] << "    " << RAT[i] << "\n";
    }

    cout << "------------------------------------------\n";
    // Printing Instruction Queue
    cout << "Instuction Queue\n";
    for (int i = 0; i < instruction_queue.size(); i++){
        instruction inst = instruction_queue[i];
        switch(inst.opcode){
            case 0:
                cout << "Add R" << inst.destination << ", R" << inst.op1 << ", R" << inst.op2 << "\n";
                break;
            case 1:
                cout << "Sub R" << inst.destination << ", R" << inst.op1 << ", R" << inst.op2 << "\n";
                break;
            case 2: 
                cout << "Mult R" << inst.destination << ", R" << inst.op1 << ", R" << inst.op2 << "\n";
                break;
            case 3:
                cout << "Div R" << inst.destination << ", R" << inst.op1 << ", R" << inst.op2 << "\n";
                break;
            default:
                continue;
        }
    }
    cout << endl;

    return ;
}

int main(int argc, char** argv){
    if (argc == 1){
        printf("Usage : ./binary <instructions filename>");
        return -1;
    }
    string filename = string(argv[1]);
    ifstream fd;
    fd.open(filename);

    vector<instruction> instruction_queue; // Instruction Queue
    vector<instruction> instruction_queue_extra; // Store extra instruction that did not fit in queue here
    vector<rs> RSs(RS_NO); // Reservation Stations
    vector<int> RAT(REG_NO); // Register Allocation Table
    vector<int> RF(REG_NO); // Register File
    
    ////////////////////////////////////////
    int no_of_instructions;
    int no_of_cycles;
    int instructions_pending;
    int start, end;
    instruction inst;
    //////////////////////////////////////////////
    // Initialise all relevant containers
    // Read from instruction file
    fd >> no_of_instructions;
    fd >> no_of_cycles;
    // cout << no_of_instructions << (no_of_cycles);
    instructions_pending = no_of_instructions - min(no_of_instructions,MAX_INST_QUEUE);

    // Fill the instructon buffer
    for( int i = 0; i < no_of_instructions; i++){
        
        if (i < MAX_INST_QUEUE){
            instruction tmp;
            fd >> tmp.opcode;
            fd >> tmp.destination; 
            fd >> tmp.op1; 
            fd >> tmp.op2;
            instruction_queue.push_back(tmp);
        } else {
            instruction tmp;
            fd >> tmp.opcode;
            fd >> tmp.destination; 
            fd >> tmp.op1; 
            fd >> tmp.op2;
            instruction_queue_extra.push_back(tmp);
        }
    }
    reverse(instruction_queue.begin(), instruction_queue.end());
    reverse(instruction_queue_extra.begin(), instruction_queue_extra.end());

    // Read initial register values
    for (int i = 0; i < REG_NO; i++){
        fd >> RF[i];
    }

    fd.close();
    
    // Initialise RSs and RAT 
    for(int i = 0; i < RS_NO; i++){
        RSs[i].busy = 0;
        RSs[i].disp = -1;
        RSs[i].vj = -1;
        RSs[i].vk = -1;
        RSs[i].qj = -1;
        RSs[i].qk = -1;
    }

    for (int i = 0; i < REG_NO; i++){
        RAT[i] = -1;
    }

    ///////////////////////////////////////////////////////////////
    // Simulation 
    for(int i = 0; i < no_of_cycles; i++){
        cout << "----------------------------------------\n";
        cout << "Cycle " << i << endl;
        
        // Print current system state
        printState(instruction_queue,RSs,RAT,RF);
        
        // Checking for instruction to dispatch - ADD/SUB
        for (int j = 0; j < 3; j++){
            
            if (RSs[j].busy && (RSs[j].vj != -1) && (RSs[j].vk != -1) && (RSs[j].disp == -1) ){
                // Check if execution unit is busy or not
                int tmp = false;
                for (int k = 0; k < 3; k++){
                    if ( (RSs[k].busy) && (RSs[k].disp != -1) )
                        tmp = true;
                }

                if (tmp)
                    break;

                RSs[j].disp = i;
                break;
            }

        }    

        // Checking for instruction to dispatch - MUL/DIV
        for (int j = 3; j < 5; j++){
            
            if (RSs[j].busy && (RSs[j].vj != -1) && (RSs[j].vk != -1) && (RSs[j].disp == -1) ){
                // Check if execution unit is busy or not
                int tmp = false;
                for (int k = 3; k < 5; k++){
                    if ((RSs[k].busy) && (RSs[k].disp != -1))
                        tmp = true;
                }
                
                if (tmp)
                    break;

                RSs[j].disp = i;
                break;
            }

        }    

        // Check if an instruction can be issued
        if (instruction_queue.size() != 0){
            inst = instruction_queue.back();
                
            if (inst.opcode == 0 ||  inst.opcode == 1) {
                start = 0;
                end = 3;
            } else {
                start = 3;
                end = 5;
            }

            // Issue if vacant resevation stations are left
            for (int j = start; j < end; j++){
                if (!RSs[j].busy){
                    RSs[j].opcode = inst.opcode;
                    RSs[j].busy = 1;

                    if (RAT[inst.op1] == -1)
                        RSs[j].vj = RF[inst.op1];
                    else
                        RSs[j].qj = RAT[inst.op1];

                    if (RAT[inst.op2] == -1)
                        RSs[j].vk = RF[inst.op2];
                    else
                        RSs[j].qk = RAT[inst.op2];
                    
                    RAT[inst.destination] = j;

                    instruction_queue.pop_back();
                    break;
                }
            }
        }
        
        // Execution completion
        for (int j = RS_NO-1; j >= 0; j--){
            if ( (RSs[j].busy && (RSs[j].disp != -1) ) && ( ( (RSs[j].opcode == 0 || RSs[j].opcode == 1) && (RSs[j].disp + 2 <= i) ) || ( (RSs[j].opcode == 2) && (RSs[j].disp + 10 <= i) ) || ( (RSs[j].opcode == 3) && (RSs[j].disp + 40 <= i) ) ) ){
                int tmp;
                switch (RSs[j].opcode){
                    case 0:
                        tmp = RSs[j].vj + RSs[j].vk;
                        break;
                    case 1:
                        tmp = RSs[j].vj - RSs[j].vk;
                        break;
                    case 2:
                        tmp = RSs[j].vj * RSs[j].vk;
                        break;
                    case 3:
                        tmp = RSs[j].vj / RSs[j].vk;
                        break;
                    default:
                        continue;
                }
                
                // Capturing
                for (int k = 0; k < RS_NO; k++){
                    if( (k != j) && ( RSs[k].busy && ( (RSs[k].qj == j) || (RSs[k].qk == j)) )){
                        if(RSs[k].qj == j){
                            RSs[k].qj = -1;
                            RSs[k].vj = tmp;
                        }

                        if(RSs[k].qk == j){
                            RSs[k].qk = -1;
                            RSs[k].vk = tmp;
                        }
                    }
                }
                
                // Commit
                for (int k = 0; k < REG_NO; k++){
                    if (j == RAT[k]){
                        RF[k] = tmp;
                        RAT[k] = -1;
                        break;
                    }
                }
            
                // Clearing the completed instruction entry in RS
                RSs[j].busy = 0;
                RSs[j].disp = -1;
                RSs[j].vj = -1;
                RSs[j].vk = -1;
                RSs[j].qj = -1;
                RSs[j].qk = -1;

                break;
            }
        }

        // Load new instruction into the instruction buffer
        if (instruction_queue_extra.size() != 0){
            inst = instruction_queue_extra.back();
            instruction_queue.insert(instruction_queue.begin(),inst);
            instruction_queue_extra.pop_back();
        }

        
    }

}