#define _GLIBCXX_USE_CXX11_ABI 0
#include <bits/stdc++.h>
using namespace std;

int register_values[32]={0};
string REG[32]={"zero","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","fp","ra"};


string pipeline[100][500];



//initialise all values of pipeline to null;

/*
int clock = 0;
int stallsNum = 0;
class pipeline{
    public:
        int dataForwd = 0;
        string curr_ins;
        void IF(){
            clock++;
        }
        void ID_RF(){
        }
        void EX(){
            if(dataForwd == 1){
            }
        }
        void MEM(){
        }
        
        void WB(){
        }
};*/


class mipsSimulator{
public:
    int MEM[1024]={0};
       int programCounter;
       int NumberOfInstructions;
       int clock,branch_flag;
       int MaxLength;//10000
       vector<string> InputProgram; //to store the input program
       struct Memoryword{
           string value;
           string address;//pc line number
        };
        struct Label{
           string labelname;
           string address;
        };
        vector<struct Memoryword>Mem;
        vector<struct Label>labeltable;

        mipsSimulator(string fileName){
        programCounter=0;
        NumberOfInstructions=0;
        MaxLength=10000;
        clock=0;
        branch_flag=0;
        ifstream InputFile;
        InputFile.open(fileName.c_str(),ios::in); //open file
        if(!InputFile){ //if open failed
            cout<<"Error: File does not exist or could not be opened"<<endl;
            exit(1);
        }
        string tempString;
        while(getline(InputFile,tempString)){ //read line by line
        //readInstruction(tempString);
            NumberOfInstructions++;
            if(NumberOfInstructions>MaxLength){ ///check number of instructions with maximum allowed
                cout<<"Error: Number of lines in input too large, maximum allowed is "<<MaxLength<<" line"<<endl;
                exit(1);
            }
            InputProgram.push_back(tempString); //store in InputProgram
        }
        InputFile.close();
        }
        string readInstruction(string str){
            if(str.find("#")!=-1){ //remove comments
                    str=str.substr(0,str.find("#"));
                    }
            str.erase(remove(str.begin(), str.end(), ' '), str.end());
            str.erase(remove(str.begin(), str.end(), ','), str.end());
            str.erase(remove(str.begin(), str.end(), '$'), str.end());
           return str;
        }

        void reportError(int line_number){
            cout<<"Error found in :"<<(line_number+1)<<": "<<InputProgram[line_number]<<endl;
        }

        void preprocess(){
            int i=0,j=0;
            int current_section=-1; //current_section=0 - data section, current_section=1 - text section
            int index; //to hold index of ".data"
            int flag=0; //whether "..data" found
            //string current_instruction="";
            int dataStart=0; //line number for start of data section
            int textStart=0;
            for(i=0;i<NumberOfInstructions;i++){
                string current_instruction="";
                current_instruction=InputProgram[i];
                current_instruction = readInstruction(current_instruction);
                index=current_instruction.find(".data");
                if(index==-1)
                continue;
                else if(flag==0){
                    flag=1;
                    current_section=0;
                    dataStart=i;
                }
                else if(flag==1){
                    cout<<"Multiple instances of .data found"<<endl;
                    exit(1);
                }
            }
            int wordindex,arrayindex;
            if(current_section==0){
                for(i=dataStart+1;i<NumberOfInstructions;i++){
                    string current_instruction="";
                    current_instruction=InputProgram[i];
                    current_instruction = readInstruction(current_instruction);
                    arrayindex=current_instruction.find(":");//array:.word9315
                    wordindex=current_instruction.find(".word");
                    int storeline;
                    if(wordindex==-1 && arrayindex==-1){
                        if(current_instruction.find(".text")==-1){ //if text section has not started
                            cout<<"Error: Unexpected symbol in data section"<<endl;
                        }
                        else{
                            break;
                        }
                    }
                    else{
                        string num=current_instruction.substr(arrayindex+6);//array:.word9135
                        //lets assume array values are <10
                        int k=0;
                        for(int i=0;i<num.length();i++){
                            MEM[k]=stoi(num.substr(i,1));
                            k++;
                        }
                    } 
                }
            }
            int textIndex=0;
            int textFlag=0;

            for(i=programCounter;i<NumberOfInstructions;i++)
            {
                string current_instruction=InputProgram[i];
                current_instruction = readInstruction(current_instruction);
                if(current_instruction=="")
                {
                    continue;
                }
                textIndex=current_instruction.find(".text"); //find text section similar as above
                if(textIndex==-1)
                {
                    continue;
                }
                else if(textFlag==0)
                {
                    textFlag=1;
                    current_section=1;
                    textStart=i;
                }
                else if(textFlag==1)
                {
                    cout<<"Error: Multiple instances of .text"<<endl;
                    reportError(i);
                }
            }
            if(current_section!=1) //if text section not found
            {
                cout<<"Error: Text section does not exist or found unknown string"<<endl;
                exit(1);
            }
            if(InputProgram[textStart+1]!=".globl main"){
                cout<<"Error: No (.globl main) found"<<endl;
                exit(1);
            }
            int foundmain=0;
            int main_index=0,labelindex=-1;
            if(InputProgram[textStart+2]!="main:"){
                cout<<"Error: No main found"<<endl;
                exit(1);
            }
            else{
                foundmain=1;
                main_index=textStart+2;
            }
            for(int i=main_index+1;i<NumberOfInstructions;i++){
                string current_instruction=InputProgram[i];
                current_instruction = readInstruction(current_instruction);
                labelindex=current_instruction.find(":");
                if(labelindex==0){
                    cout<<"Error : Label name expected"<<endl;
                    reportError(i);
                }
                else if(labelindex==-1){
                    continue;
                }
                else{
                    j=labelindex-1;
                    string temp="";
                    temp=current_instruction.substr(0,j+1);
                    Label templabel;
                    templabel.labelname=temp;
                    templabel.address=to_string(i+1);
                    labeltable.push_back(templabel);
                }
            }
            for(i=0;labeltable.size()>0 && i<(labeltable.size()-1);i++) //check for duplicates
            {
                if(labeltable[i].labelname==labeltable[i+1].labelname)
                {
                    cout<<"Error: One or more labels are repeated"<<endl;
                    exit(1);
                }
            }
        }


        void processInstruction(string current_instruction){
            if(current_instruction.substr(0,3)=="add" && current_instruction.substr(3,1)!="i"){
                int reg_store[3]={-1};
                for(int i=0;i<32;i++){
                    if(current_instruction.substr(3,2)==REG[i])
                        reg_store[0]=i;
                    if(current_instruction.substr(5,2)==REG[i])
                        reg_store[1]=i;
                    if(current_instruction.substr(7,2)==REG[i])
                        reg_store[2]=i;
                }
                register_values[reg_store[0]]= register_values[reg_store[1]]+ register_values[reg_store[2]];
                programCounter++;
                return;
            }
            if(current_instruction.substr(0,3)=="sub"){
                int reg_store[3]={-1};
                for(int i=0;i<32;i++){
                    if(current_instruction.substr(3,2)==REG[i])
                        reg_store[0]=i;
                    if(current_instruction.substr(5,2)==REG[i])
                        reg_store[1]=i;
                    if(current_instruction.substr(7,2)==REG[i])
                        reg_store[2]=i;

                }
               
                     register_values[reg_store[0]]= register_values[reg_store[1]]-register_values[reg_store[2]];
                      programCounter++;
                      return;
            }
            if(current_instruction.substr(0,3)=="mul"){
                int reg_store[3]={-1};
                for(int i=0;i<32;i++){
                    if(current_instruction.substr(3,2)==REG[i])
                        reg_store[0]=i;
                    if(current_instruction.substr(5,2)==REG[i])
                        reg_store[1]=i;
                    if(current_instruction.substr(7,2)==REG[i])
                        reg_store[2]=i;
                }        
                     register_values[reg_store[0]]= register_values[reg_store[1]]*register_values[reg_store[2]];
                      programCounter++;
                      return;
            }
            if(current_instruction.substr(0,3)=="div"){
                int reg_store[3]={-1};
                for(int i=0;i<32;i++){
                    if(current_instruction.substr(3,2)==REG[i])
                        reg_store[0]=i;
                    if(current_instruction.substr(5,2)==REG[i])
                        reg_store[1]=i;
                    if(current_instruction.substr(7,2)==REG[i])
                        reg_store[2]=i;

                }
               
                     register_values[reg_store[0]]= register_values[reg_store[1]]/register_values[reg_store[2]];
                      programCounter++;
                      return;
            }


            if(current_instruction.substr(0,4)=="addi"){//addit2t34
                string rs,rd,imm;
                int immediate;
                rd=current_instruction.substr(4,2);
                if(current_instruction.substr(6,2)!="ze"){
                    rs=current_instruction.substr(6,2);
                    imm=current_instruction.substr(8);
                    immediate=stoi(imm);
                }
                else{
                     rs=current_instruction.substr(6,4);
                     imm=current_instruction.substr(10);
                    immediate=stoi(imm);
                }
                int reg_store[2]={-1};
                for(int i=0;i<32;i++){
                    if(rd==REG[i])
                        reg_store[0]=i;
                    if(rs==REG[i])
                        reg_store[1]=i;
                }
                register_values[reg_store[0]]=immediate+register_values[reg_store[1]];
                programCounter++;
                return;
            }

            
            
            
            if(current_instruction.substr(0,3)=="beq"){
                string st;
                int reg_store[2]={-1};
                if(current_instruction.substr(5,2)=="ze"){
                    for(int i=0;i<32;i++){
                        if(current_instruction.substr(3,2)==REG[i])
                            reg_store[0]=i;
                        if(current_instruction.substr(5,4)==REG[i]) //beqt0zeroLABEL
                            reg_store[1]=i;
                    }
                    st = current_instruction.substr(9);
                }
                else{
                    for(int i=0;i<32;i++){
                        if(current_instruction.substr(3,2)==REG[i])
                            reg_store[0]=i;
                        if(current_instruction.substr(5,2)==REG[i])
                            reg_store[1]=i;
                    }
                    st = current_instruction.substr(7);
                }

                string addr;
                for(int i=0;i<labeltable.size();i++){
                    if(labeltable[i].labelname==st){
                        addr=labeltable[i].address;
                    }
                }
                if(register_values[reg_store[0]]==register_values[reg_store[1]]){
                    programCounter=stoi(addr) + 1;
                }
                else{
                    programCounter++;
                }
                return;
            }

            if(current_instruction.substr(0,3)=="bne"){
                string st;
                int reg_store[2]={-1};
                if(current_instruction.substr(5,2)=="ze"){  //bnet2t3LABEL
                    for(int i=0;i<32;i++){
                        if(current_instruction.substr(3,2)==REG[i])
                            reg_store[0]=i;
                        if(current_instruction.substr(5,4)==REG[i]) //beqt0zeroLABEL
                            reg_store[1]=i;
                    }
                    st = current_instruction.substr(9);
                }
                else{
                    for(int i=0;i<32;i++){
                        if(current_instruction.substr(3,2)==REG[i])
                            reg_store[0]=i;
                        if(current_instruction.substr(5,2)==REG[i])
                            reg_store[1]=i;
                    }
                    st = current_instruction.substr(7);
                }
                string addr;
                for(int i=0;i<labeltable.size();i++){
                    if(labeltable[i].labelname==st){
                        addr=labeltable[i].address;
                    }
                }
                if(register_values[reg_store[0]]!=register_values[reg_store[1]]){
                    programCounter=stoi(addr) + 1;
                }
                else{
                    programCounter++;
                }
                return;
            }

            if(current_instruction.substr(0,1)=="j" && current_instruction.substr(1,1)!="r"){
              
                string st = current_instruction.substr(1);
                string addr;
                for(int i=0;i<labeltable.size();i++){
                    if(labeltable[i].labelname==st){
                        addr=labeltable[i].address;
                    }
                }
               programCounter=stoi(addr) + 1;
               return;

            }
           if(current_instruction.substr(0,2)=="lw"){
                  string rd,rs,offset;
                  rd=current_instruction.substr(2,2);
                  int index=current_instruction.find("(");
                  rs=current_instruction.substr(index+1,2);
                  offset=current_instruction.substr(4,index-4);
                int offs = stoi(offset);
                int value;
                int reg_store[2]={-1};
                for(int i=0;i<32;i++){
                    if(rs==REG[i])
                        reg_store[0]=i;
                    else if(rd==REG[i])
                        reg_store[1]=i;
                }
                value = register_values[reg_store[0]];
                register_values[reg_store[1]] = MEM[(offs + value)/4];
                programCounter++;
                return;
             }
            if(current_instruction.substr(0,2)=="sw"){
                  string rd,rs,offset;
                  rs=current_instruction.substr(2,2);
                  int index=current_instruction.find("(");
                  rd=current_instruction.substr(index+1,2);
                  offset=current_instruction.substr(4,index-4);
                int offs = stoi(offset);
                int value;

                int reg_store[2]={-1};
                for(int i=0;i<32;i++){
                    if(rs==REG[i])
                        reg_store[0]=i;
                    else if(rd==REG[i])
                        reg_store[1]=i;
                }                                     
                value = register_values[reg_store[1]];
                MEM[(offs + value)/4]=register_values[reg_store[0]];
                programCounter++;
                return;
            }
            if(current_instruction.substr(0,3)=="slt"){
                string rd,src1,src2;
                rd=current_instruction.substr(3,2);
                src1=current_instruction.substr(5,2);
                src2=current_instruction.substr(7,2);
                int reg_store[3]={-1};
                for(int i=0;i<32;i++){
                    if(src1==REG[i])
                    reg_store[0]=i;
                    else if(src2==REG[i])
                    reg_store[1]=i;
                    else if(rd==REG[i])
                    reg_store[2]=i;
                }
                if(register_values[reg_store[0]]<register_values[reg_store[1]]){
                    register_values[reg_store[2]]=1;
                }
                else{
                     register_values[reg_store[2]]=0;
                }
                programCounter++;
                return;
            }
            if(current_instruction.substr(0,2)=="la"){ //las0array
                for(int i=0;i<32;i++){
                    if(current_instruction.substr(2,2)==REG[i]){
                        register_values[i]=0; //MEM[0]=0;
                        break;
                    }
                }
                programCounter++;
                return;
            }
            if(current_instruction.substr(0,2)=="jr"){
                programCounter++;
                return;
            }
        }








        void fill(int x,int y,int iF, int id, int ex, int mem, int wb){
            for(int k=y; k<y+iF; k++){
                pipeline[x][k]="stall";
            }
            pipeline[x][y]="IF";    
            y++;
            for(int k=y; k<y+id; k++){
                pipeline[x][k]="stall";
            }
            pipeline[x][y]="ID";
            y++;
            for(int k=y; k<y+ex; k++){
                pipeline[x][k]="stall";
            }
            pipeline[x][y]="EX";
            y++;
            for(int k=y; k<y+mem; k++){
                pipeline[x][k]="stall";
            }
            pipeline[x][y]="MEM";
            y++;
            for(int k=y; k<=y+wb; k++){
                pipeline[x][k]="stall";
            }
            pipeline[x][y]="WB";
        }


        string hazard(string ins){
            if(ins.substr(0,4)=="addi"){
                return ins.substr(4,2);
            }
            if(ins.substr(0,3)=="add" && (ins.substr(3,1)!="i")){
                return ins.substr(3,2);
            }
            if(ins.substr(0,3)=="sub"){
                return ins.substr(3,2);
            }
            if(ins.substr(0,3)=="mul"){
                return ins.substr(3,2);
            }
            if(ins.substr(0,3)=="div"){
                return ins.substr(3,2);
            }
            if(ins.substr(0,3)=="slt"){
                return ins.substr(3,2);
            }
            if(ins.substr(0,2)=="lw"){
                return ins.substr(2,2);
            }
            if(ins.substr(0,2)=="sw"){
                return ins.substr(2,2);
            }


            //if ..... other functions
        }
        bool branchhazard(string ins){
            bool flag=false;
             if(ins.substr(0,3)=="beq"||ins.substr(0,3)=="bne"||(ins.substr(0,1)=="j"&&ins.substr(1,1)!="r")){
                flag=true;
            }
            return flag;
        }
        //fill(i,i,000000)
        void stalls_hazard(int ins_row){
            int IF,ID,EX,MEM;
            int clk_len=0;
            for(int i=0;i<500;i++){
                if(pipeline[ins_row][i]=="WB")
                  clk_len=i;
            }
            for(int i=0;i<clk_len;i++){
              if(pipeline[ins_row][i]=="IF")
              IF=i;
               if(pipeline[ins_row][i]=="ID")
              ID=i;
               if(pipeline[ins_row][i]=="EX")
              EX=i;
               if(pipeline[ins_row][i]=="MEM")
              MEM=i;
            }
           // int cnt1=0,cnt2=0,cnt3=0,cnt4=0;
            for(int i=IF+1;i<ID;i++){
                if(pipeline[ins_row][i]=="stall"){
                  //fill(ins_row+1,i,0,1,0,0,0);
                  pipeline[ins_row+1][i]=="stall";

                }
            }
            for(int i=ID+1;i<EX;i++){
                if(pipeline[ins_row][i]=="stall"){
                 // fill(ins_row+1,i,0,0,1,0,0);
                  pipeline[ins_row+1][i]=="stall";
                }
            }
            for(int i=EX+1;i<MEM;i++){
                if(pipeline[ins_row][i]=="stall"){
                  //fill(ins_row+1,i,0,0,0,1,0);
                  pipeline[ins_row+1][i]=="stall";
                }
            }
            for(int i=MEM+1;i<clk_len;i++){
                if(pipeline[ins_row][i]=="stall"){
                  //fill(ins_row+1,i,0,0,0,0,1);
                  pipeline[ins_row+1][i]=="stall";
                }
            }
        }
//add
//slt
//sub 
//mul
//div

//bne
//beq
//j
//lw
//jr
        void fillPipeline(int numb_rows,int flagForwdg){
            int j;
            for(int i=0; i<numb_rows; i++){
                j=clock+1;
                //add t1 t2 t3
                //add t3 t1 t1

                if(pipeline[i][0].substr(0,4)=="addi"){
                    if(i!=0 && pipeline[i][0].substr(6,2) == hazard(pipeline[i-1][0])){
                         if(flagForwdg==0){//no forwarding
                           if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                        }
                        else{//with forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);


                        }
                        clock++;
                    }
                    else{
                         if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

                if(pipeline[i][0].substr(0,3)=="add"){
                    if(pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(7,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                            
                        }
                        else{//with forwarding
                             if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                         if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

                if(pipeline[i][0].substr(0,3)=="sub"){
                    if(pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(7,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                             if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                        }
                        else{//with forwarding
                             if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                       if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

                if(pipeline[i][0].substr(0,3)=="mul"){
                    if(pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(7,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                           if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                        }
                        else{//with forwarding
                           if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

                if(pipeline[i][0].substr(0,3)=="div"){
                    if(pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(7,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                        }
                        else{//with forwarding
                           if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

                if(pipeline[i][0].substr(0,3)=="slt"){
                    if(pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(7,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                        }
                        else{//with forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }
                if(pipeline[i][0].substr(0,3)=="beq"){
                     int pc;
                     branch_flag=0;
                     for(int j=0;j<NumberOfInstructions;j++){
                         if(pipeline[i][0]==InputProgram[j])
                            pc=i+1;                         
                     }
                     if(pipeline[i+1][0]!=InputProgram[pc+1])
                     branch_flag=1;
                     else
                     branch_flag=0;
                    if(pipeline[i][0].substr(3,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0])){
                       if(flagForwdg==0){
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                       }
                       else{
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                       }
                       clock++;
                    }
                     else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }

                    

                }
                 if(pipeline[i][0].substr(0,3)=="bne"){
                    int pc;
                     branch_flag=0;
                     for(int i=0;i<NumberOfInstructions;i++){
                         if(pipeline[i][0]==InputProgram[i])
                            pc=i+1;
                     }
                     if(pipeline[i+1][0]!=InputProgram[pc+1])
                     branch_flag=1;
                     else
                     branch_flag=0;
                     if(pipeline[i][0].substr(3,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0])){
                       if(flagForwdg==0){
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                       }
                       else{
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                       }
                       clock++;
                    }
                     else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }


                }
                 if(pipeline[i][0].substr(0,1)=="j" && pipeline[i][0].substr(1,1)!="r"){
                    int pc;
                     branch_flag=0;
                     for(int i=0;i<NumberOfInstructions;i++){
                         if(pipeline[i][0]==InputProgram[i])
                            pc=i+1;
                     }
                     if(pipeline[i+1][0]!=InputProgram[pc+1])
                     branch_flag=1;
                     else
                     branch_flag=0;
                     if(pipeline[i][0].substr(3,2) == hazard(pipeline[i-1][0]) || pipeline[i][0].substr(5,2) == hazard(pipeline[i-1][0])){
                       if(flagForwdg==0){
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                            stalls_hazard(i-1);
                       }
                       else{
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                       }
                       clock++;
                    }
                     else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }
                  if(pipeline[i][0].substr(0,2)=="lw"){
                    if(i!=0 && pipeline[i][0].substr(pipeline[i][0].length()-3,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                        }
                        else{//with forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

                if(pipeline[i][0].substr(0,2)=="sw"){
                    if(i!=0 && pipeline[i][0].substr(pipeline[i][0].length()-3,2) == hazard(pipeline[i-1][0])){
                        if(flagForwdg==0){//no forwarding
                            if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,2,0,0);
                            else
                            fill(i,j,0,0,2,0,0);
                        }
                        else{//with forwarding
                           if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                            else
                            fill(i,j,0,0,0,0,0);
                        }
                        clock++;
                    }
                    else{
                        if(branchhazard(pipeline[i-1][0]) && branch_flag==1)
                            fill(i,j,1,0,0,0,0);
                        else
                        fill(i,j,0,0,0,0,0);
                        clock++;
                    }
                }

            }
        }


        






        void display(){
           cout<<"Registers:"<<"        "<<"Value:"<<endl;
           cout<<endl;
           for(int i=0;i<32;i++){
               cout<<REG[i]<<"                 "<<register_values[i]<<endl;
           }
        }

        void execute(){
            /*
            for(int i=0;i<100;i++){
                for(int j=0;j<500;j++){
                    pipeline[i][j] = "null";
                }
            }*/


            preprocess();
            int mainindex;
            for(int i=1;i<=NumberOfInstructions-1;i++){
                if(InputProgram[i]=="main:"){
                mainindex=i;
                break;
                }
            }
            programCounter=mainindex+2;

            /*
            if(mode==1){
            while(programCounter<=NumberOfInstructions){
                string current_instruction = readInstruction(InputProgram[programCounter-1]);
                 cout << "ProgramCounter:" << programCounter << endl;
                processInstruction(current_instruction);
                cout<<"MEMORY:"<<endl;
                for(int i=0;i<1024;i++){
                    if(MEM[i]!=0){
                         cout<<MEM[i]<<" ";
                    }
                }
                cout<<endl;
            }
            //cout << "ProgramCounter:" << programCounter << endl;
            cout<<endl<<endl;
            display();
            return;
            }
            */

            //else{
                int flag; 
            int pipeRow = 0;
            while(programCounter<=NumberOfInstructions){
                string current_instruction = readInstruction(InputProgram[programCounter-1]); 
                processInstruction(current_instruction);
                //cout << current_instruction << endl;
                pipeline[pipeRow][0]=current_instruction;
                pipeRow++;
            }
            fillPipeline(pipeRow, flag);
            cout << "ProgramCounter:" << programCounter << endl;
            cout<<endl;
            cout<<endl;
            display();
            cout<<endl;
            cout<<endl;
             cout<<"MEMORY:"<<endl;
                for(int i=0;i<1024;i++){
                    if(MEM[i]!=0){
                         cout<<MEM[i]<<" ";
                    }
                }
                cout<<endl;
            return;
            //}
        }
};
int main(){
    cout<<"Welcome to Team dynamic MIPS SIMULATOR!!"<<endl;
    /*int mode;
    cout<<"Enter mode 1 or 2:      1.Step-bystep execution   2.Final output"<<endl;
    cin>>mode;*/
    //mode 2 is changed to nly print the current instruction.

    //mipsSimulator simulator("mipsBubblesort.asm");
    mipsSimulator simulator("BubbleSort.asm");




    /*
    cout << "--------------------------";
    vector<string> instructionss = simulator.InputProgram;
    for(int i=0; i < instructionss.size(); i++){
        std::cout << instructionss.at(i) << endl;
    }
    cout << "--------------------------";
    */
    simulator.execute();
}
