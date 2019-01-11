#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
void print_exp(char* c, int* s, int* s_size, int* t_index);
int* decToBinary(int n);
void print_mult(int* bin, int s_index, int * t_index);
int ispoweroftwo(int x);
int getpower(int x);
int main(int argc, char *argv[]){
    //Algorithm: Read entire file in as a string
    //           Split string into expressions delimited by semicolons
    //           Send each char* (OR store as equation[] locally)
    //           Process equation, print required things
    //Variables that need to be memorized between lines: s[], s_size
    int s[8]; //Store s register values to corresponding indices, does not reset
    char buff[128];
    char subbuff[64];

    FILE* fp = fopen(argv[1], "r");

    int index = 0;
    int s_size = 0; //Functions as global variable to be kept track of
    int t_index = 0;//Same as above

    memset(s,0,sizeof(s)); //Initializing s and subbuff
    memset(subbuff, 0 , sizeof(subbuff));

    while(fgets(buff, 128, fp)){

        while(buff[index] != ';'){ //Find size of expression
            index++;
        }

        memcpy(subbuff, &buff[0], index); //Cut out size of expression
        subbuff[index] = '\0'; //Set null terminator to set length of string
        print_exp(subbuff, s, &s_size, &t_index); //Send "global" variables by reference to allow changes.
        index = 0;
    }
    fclose(fp);
    return EXIT_SUCCESS;
}
void print_exp(char* input, int* s, int* s_size, int* t_index){
    printf("# %s;\n", input);
    int temp;
    int equation[128]; //Store equation as an int array to grab multi-digit values
    
    int i; int j = 0;
    int ops[128]; int ops_size = 0; 
    int constants[128]; int const_size = 0; //Store constants as int to work with multi digit values
    
    int eq_size = 0; 
    int eq_index = 0;
    
    int s_index = 0;
    int found; 
    int repeat = 0;
    while(sscanf(input, "%s",(char*) &temp)==1){ //Scan strings separated by spaces, store in temp for this loop.
            
            input+=(strlen((char*)&temp)+1);
            repeat = 0;

        if(isdigit((unsigned char)temp) || (((char*)&temp)[0] == '-' && isdigit(((char*)&temp)[1]))){//If temp is a number, add it to the constants and equation array, and increment both.
            
            constants[const_size] = atoi((char*)&temp);
            equation[eq_size] = atoi((char*)&temp);
            eq_size++;
            const_size++;

        }else if(!isalnum((unsigned char)temp) && (unsigned char)temp != '='){ //If temp is an operator and NOT '=', add it to the ops array and increment.
            
            ops[ops_size] = temp;
            ops_size++;

        }else if(isalnum((unsigned char)temp) && islower((unsigned char)temp)){ //Temp is not a digit or operator at this point, so if it is a char, then it will be cross checked
            
            for(i = 0; i < *s_size; i++){        //against the existing array of variables stored in s
                if((unsigned char)s[i] == (unsigned char)temp){
                    repeat = 1;
                }
            }

            if(repeat == 0){ //If temp is not a variable seen before, add it to the s register
            s[*s_size] = temp; //Place variables in corresponding registers.
            (*s_size)++;
            }
            equation[eq_size] = temp;
            eq_size++;
        }
        
    }

    int swap = 0;
    int L_index = 0;

    if(ops_size == 0){
        printf("li $s%d,%d\n", *s_size-1, constants[0]);
    }else{
        eq_index = 1;
        for(j = 0; j < ops_size; j++){ //ops_size = number of operations, which is the number of prints
         
            if(j==0){
                found = 0; //Found is the number of registers needed for a full print.  So the first iteration requires two different registers,
            }else{     //but every print after that requires one value so found begins at 1 when j>0
                found = 1;
            }
            char temp = ops[j];
            
            if(temp == '+'){
                printf("add ");

            }else if(temp == '*'){
                int isint = 0;
                for(i = 0; i < const_size; i++){
                    if(equation[eq_index+1] == constants[i]){
                        isint = 1;
                    }
                }
                if(isint){ //Check if being multiplied by a constant.
                    for(i = 0; i < *s_size; i++){
                        if((unsigned char)s[*s_size] == (unsigned char)equation[eq_index]){
                            s_index = i;
                        }
                    }

                    if(equation[eq_index+1] == 1 || equation[eq_index+1] == -1){
                        printf("move $t%d,$s%d\n",*t_index, (*s_size)-2);
                        
                        if(equation[eq_index+1] == 1){
                            printf("move $s%d,$t%d\n",(*s_size)-1,*t_index);
                        }else{
                            printf("sub $s%d,$zero,$t%d\n", (*s_size)-1, *t_index);
                        }

                        (*t_index)++;
                        eq_index++;
                        continue;

                    }else if(equation[eq_index+1] == 0){

                        for(i = 0; i < *s_size; i++){
                            if((unsigned char)s[*s_size] == (unsigned char)equation[eq_index]){
                                s_index = i;
                            }
                        }
                        printf("li $s%d,0\n", s_index );

                    }else{
                        print_mult(decToBinary(equation[eq_index+1]), s_index, t_index);
                        (*t_index)++;
                        eq_index++;
                    }
                    continue;

                }else{
                    printf("mult ");
                   // if(ops_size != 1) (*t_index)++; //Increment if not an assignment statement
                }
            swap = 1;
        }else if(temp == '/' || temp == '%'){
            
            int isint = 0;

            for(i = 0; i < const_size; i++){
                if(equation[eq_index+1] == constants[i]){
                    isint = 1;
                }
            }

            if(isint){
                int isneg = (equation[eq_index+1] < 0 ? 1 : 0);
                int abs_val;
                if(isneg){
                    abs_val = (equation[eq_index+1]*-1);
                }else{
                    abs_val  = equation[eq_index+1];
                }
                if(ispoweroftwo(abs_val)){

                    printf("bltz $s%d,L%d\n", s_index, L_index);
                    printf("srl $s%d,$s%d,%d\n", s_index+1, s_index, getpower(abs_val));
                    L_index++;

                    if(equation[eq_index+1] < 0){
                        printf("sub $s%d,$zero,$s%d\n",s_index+1, s_index+1);
                    }

                    printf("j L%d\n", L_index);
                    printf("L%d:\n", L_index-1);
                    printf("li $t%d,%d\n", *t_index, equation[eq_index+1]);
                    printf("div $s%d,$t%d\n", s_index, *t_index);
                    printf("mflo $s%d\n", s_index+1);
                    printf("L%d:\n", L_index);

                    eq_index++;
                    continue;
                }
                if(equation[eq_index+1] == -1){
                    for(i = 0; i < *s_size; i++){
                        if((unsigned char)s[*s_size] == (unsigned char)equation[eq_index]){
                            s_index = i;
                        }
                    }
                    printf("sub $s%d,$zero,$s%d\n", (*s_size)-1, s_index+1);
                    eq_index++;
                    continue;
                }
                if(equation[eq_index+1] == 1){
                    for(i = 0; i < *s_size; i++){
                        if((unsigned char)s[*s_size] == (unsigned char)equation[eq_index]){
                            s_index = i;
                        }
                    }
                    printf("move $s%d,$s%d\n", *s_size-1, (*s_size)-2);
                    eq_index++;
                    continue;
                }
                printf("li $t%d,%d\n", (*t_index), equation[eq_index+1]);
                for(i = 0; i < *s_size; i++){
                    if((unsigned char)s[*s_size] == (unsigned char)equation[eq_index]){
                        s_index = i;
                    }
                }

                printf("div $s%d,$t%d\n", s_index, (*t_index));
                (*t_index)++;
                printf("mflo $t%d\n", (*t_index));
                eq_index++;
                continue;
                }else{
                    printf("div ");
                    swap = 1;
                    
                }
        }else{
            printf("sub ");
        }
        if(!swap){
            if(j == (ops_size-1)){  //Corner case for assigning final operation to $s0
                printf("$s%d,", (*s_size)-1);
                (*t_index)--;
            }else{
                printf("$t%d,", *t_index);
            }
            if(j != (ops_size-1) && j > 0){
                printf("$t%d,", (*t_index)-1); //Prints $t0 and so forth as the second value in each print.
                
            }else if(j == (ops_size-1) && ops_size != 1){
                printf("$t%d,", (*t_index));
            }
             (*t_index)++;
        }else{
            if(j != 0){
                printf("$t%d,", (*t_index)); //Prints $t0 and so forth as the second value in each print.
            }
        }
        //Look at current spot in equation, loop through s register and constants to find which it matches
        int const_index = 0;
        label: //This label will be used to reloop through these for loops incase the first element to the right of the equal sign is a constant.
        for(s_index = 0; s_index < (*s_size); s_index++){
            if((unsigned char)s[s_index] == (unsigned char)equation[eq_index]){ //If there's a match, move to next element in equation, print position and add one to found
                printf("$s%d", s_index);
                eq_index++;
                s_index = 0;
                found++;
                if(found == 2){
                    break;
                }else{
                    printf(",");
                }
            }
        }
        if(found < 2){ //Checks for` matches on current equation index to any constants seen in the equation.
            for(const_index = 0; const_index < const_size; const_index++){
                if((int)constants[const_index] == (int)equation[eq_index]){

                    printf("%d", constants[const_index]);
                    eq_index++;
                    found++;
                    const_index = 0;

                    if(found ==2){
                        break;
                    }else{
                        printf(",");
                    }

                }
            }
        }
        if(found < 2) goto label;
        printf("\n");
        if(swap){

            if(j == (ops_size-1)){
                printf("mflo $s%d\n", (*s_size)-1);
            }else{
                printf("mflo $t%d\n", (*t_index));
            }
            
          //  eq_index++;
        }
    }
    
    }
}
int ispoweroftwo(int x){
    return (x != 0) && ((x & (x - 1)) == 0) && (x != 1);
}
int getpower(int x){
    int power = 1;
    int product = 2;
    while(x != product){
        product *= 2;
        power++;
    }
    return power;
}
void print_mult(int* bin, int s_index, int* t_index){
    int i = 0;
    int bin_size = 0;
    while(bin[bin_size] != 2 && bin[bin_size] != 3){
        bin_size++;
    }
    while(bin[i] != 2 || bin[i] != 3){
        if(bin[i] == 1){
            if(i != bin_size-1){
                printf("sll $t%d,$s%d,%d\n", *t_index, s_index, bin_size-i-1);
            }
            if(i == 0){
                printf("move $t%d,$t%d\n", (*t_index)+1, *t_index);
            }else if(i != bin_size-1){
                printf("add $t%d,$t%d,$t%d\n", (*t_index)+1, (*t_index)+1, (*t_index));
            }else{
                break;
            }
            
        }
        
        i++;
    }
    printf("add $t%d,$t%d,$s%d\n", (*t_index)+1, (*t_index)+1, s_index);
    if(bin[i+1] == 2){
        printf("move $s%d,$t%d\n", s_index+1, (*t_index)+1);
    }else{
        printf("sub $s%d,$zero,$t%d\n", s_index+1, (*t_index)+1);
    }
    free(bin);
}
int* decToBinary(int n){
    int* bin = (int*)calloc(sizeof(int), 32);
    int size = 0;
    int isnegative = (n < 0 ? 3 : 2);
    int dec;

    if(isnegative == 3){
        dec = n*-1;
    }else{
        dec = n;
    }

    while(dec > 0){
        bin[size] = dec % 2;
        dec /= 2;
        size++;
    }
    
    for (int i = 0; i < size/2; ++i)
    {
        int temp = bin[i];
        bin[i] = bin[size - 1 - i];
        bin[size - 1 - i] = temp;
    }

    bin[size] = isnegative; //Functions as null terminator 
    return bin;

}