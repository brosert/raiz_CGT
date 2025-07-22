#include "transactions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//// NOTE: THERE MAY BE ORDER ISSUES IF TRANSACTIONS AREN'T ENTERED CHRONOLOGICALLY
// IN PARTICULAR IF A NEW TRANSACTION IS ADDED ON A DATE THAT ALREADY HAS TRANSACTIONS THERE MAY
// BE MINOR DISCREPANCY (Actually there shouldn't - think about it)

// ADD TO QUEUE - IF WE KNOW THIS TRANSACTION HAS A LATER DATE THAN WHERE WE'RE UP TO, CHUCK IT IN

// ADD TO QUEUE ENURING DATE ORDER - WE DON'T KNOW IF THIS IS A NEWER TRANSACTION, POTENTIALLY CHECKS *EVERYU
// TRANSACTION WHICH CAN BE SLOW IF THERE ARE LARGE VOLUMES OF TRANSACTIONS


static char **gargs;

static int usage(char *errMesg) // consider expanding witrh varargs
{
    if(errMesg)
    {
        (void)fprintf(stderr, "ERROR: [%s]\n\n",errMesg);
    }
    printf("\t%s <filename>\n\n", gargs[0]);
    exit(0); // Maybe this should have an error code if we had an err  msg
}

#define IS_DIGIT(X) \
(X >= '0' && X <= '9')

#define PARSE_ERR(...) \
    { printf(__VA_ARGS__); \
    return 1; }

int get_date(char *buffer, trans_date *out_date)
{
    char *buff_ptr = buffer;
    for(int i=0; i<2; i++)
    {
        if(!IS_DIGIT(*buff_ptr))
        {
            PARSE_ERR("Invalid Date 1[%.10s]\n", buffer);
        }
        out_date->day[i] = *buff_ptr;
        buff_ptr++;
    }
    if(*buff_ptr == '/')
    {
        buff_ptr++;
    }
    else
    {
            PARSE_ERR("Invalid Date 2[%.10s]\n", buffer);
    }
    for(int i=0; i<2; i++)
    {
        if(!IS_DIGIT(*buff_ptr))
        {
            PARSE_ERR("Invalid Date 3[%.10s]\n", buffer);
        }
        out_date->month[i] = *buff_ptr;
        buff_ptr++;
    }
    if(*buff_ptr == '/')
    {
        buff_ptr++;
    }
    else
    {
            PARSE_ERR("Invalid Date 4[%.10s]\n", buffer);
    }
    for(int i=0; i<4; i++)
    {
        if(!IS_DIGIT(*buff_ptr))
        {
            PARSE_ERR("Invalid Date 5[%.10s]\n", buffer);
        }
        out_date->year[i] = *buff_ptr;
        buff_ptr++;
    }
    return 0;

}

// Modify this if we need to handle quotes with embedded commas
char *skip_past_next(char *buffer, char delim)
{
    while(*buffer && (*buffer != delim))
    {
        buffer++;
    }
    if(*buffer)
        buffer++;
    else
    {
        printf("Buffer overrun");
        return NULL;
    }
    return buffer;
}

// Add return values to detect errors
void enqueue_transaction(queued_transaction *queue, transaction_node *node)
{
    // Add date handling later
    if(queue->tail)
    {
        queue->tail->next = node;
    }
    else
    {
        if(queue->head)
        {
            printf("Uh oh, queue has bad state\n");
            exit(1);
        }
        queue->head = node;
    }
    queue->tail = node;
}

void clear_queue(queued_transaction *queue)
{
    transaction_node *curr_node = queue->head;
    while(curr_node)
    {
        queue->head = queue->head->next;
        free(curr_node->ticker);
        free(curr_node);
        curr_node = queue->head;
    }
}

void print_queue(queued_transaction *queue)
{
    transaction_node *curr_trans = queue->head;
    while(curr_trans)
    {
        printf("[%.8s: %s Units [%.8Lf] Price [%.8Lf] Cost [%.8Lf]\n", &(curr_trans->date.year), curr_trans->ticker,
                curr_trans->units, curr_trans->price, curr_trans->total);
        curr_trans = curr_trans->next;
    }
}


#define MAX_TICKER_SIZE (16) // TBH, I assume anything bigger than 4 never happens
int parse_line(char *line, queued_transaction *buys, queued_transaction *sells)
{
    char *remainder = line;
    char ticker[MAX_TICKER_SIZE] = {0x00};
    TRANSACTION_TYPE transaction_type = TT_UNKNOWN;
    int counter = 0;
    trans_date transaction_date=  {0x00}; 
    long double volume = 0.0, price = 0.0, cost = 0.0;

    if(get_date(remainder, &transaction_date))
    {
        PARSE_ERR(    "Warning invalid line [%s]\n", line);
        return 1;
    }

    if(NULL == (remainder = skip_past_next(remainder, ',')))
        PARSE_ERR("Invalid line");
    // Skip Exchange
    if(NULL == (remainder = skip_past_next(remainder, ',')))

        PARSE_ERR("Invalid line");

    if(!strncmp("BUY", remainder, 3))
    {
        transaction_type = TT_BUY;
        remainder+=3;
    }
    else if(!strncmp("SELL", remainder,4))
    {
        transaction_type = TT_SELL;
        remainder+=4;
    }
    if(NULL == (remainder = skip_past_next(remainder, ',')))
        PARSE_ERR("Invalid line");

    counter = 0;
    while(*remainder && (*remainder != ','))
    {
        ticker[counter] = *remainder;
        remainder++;
        counter++;
        if(counter >= MAX_TICKER_SIZE)
        {
            printf("Ticker too  big [%s]\n", line);
            return 1;
        }
    }
    if(*remainder) remainder++;
    if(NULL == (remainder = skip_past_next(remainder, ',')))
        PARSE_ERR("Invalid line");
        sscanf(remainder, "%Lf,%Lf,%Lf", &volume, &price, &cost);

// TEMP (?) HACK TO HANDLE IVV SPLIT ON 9/12/2022
// THINK ABOUT HOW TO HANDLE THIS MORE EFFECTIVELY/DYNAMICALLY FOR OTHER SPLITS

// NOT ACTUALLY SURE THIS IS THE RIGHT WAY TO HANDLE IT
    trans_date split_date = {0x0};
    memcpy(&split_date, "20221209", 8);
    if(0 == strncmp(ticker, "IVV", 3))
    {
        if(0 < strncmp( (char*)&split_date,(char*)&transaction_date, 8))
        {
            volume *= 15;
            price /=15;
        }
    }

// END TEMP HACK
    transaction_node *curr_trans = calloc(1, sizeof(transaction_node));
    curr_trans->ticker = strdup(ticker);
    curr_trans->date = transaction_date;
    curr_trans->units = volume;
    curr_trans->price = price;
    curr_trans->total = cost;

    if(TT_BUY == transaction_type)
    {
        enqueue_transaction(buys, curr_trans);
    }
    else if(TT_SELL == transaction_type)
    {
        enqueue_transaction(sells, curr_trans);
    }
    else
    {
        free(curr_trans->ticker);
        free(curr_trans);
        printf("Couldn't identify if sale or purchase\n[%s]\n", line);
        return 1;
    }

    return 0;
}

int isleap(int year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int monthlengths[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* return total days from January up to the end of the given month */
int monthcount(int month, int year)
{
    int r = 0;
    for(int i = 1; i <= month; i++)
        r += monthlengths[i];
    if(isleap(year) && month >= 2)
        r++;
    return r;
}

#define START_YEAR (1601)      

int makejd(int year, int month, int day)
{
    int jdnum = 0;
    jdnum += (year - START_YEAR) * 365L;
    jdnum += (year - START_YEAR) / 4;
    jdnum -= (year - START_YEAR) / 100;
    jdnum += (year - START_YEAR) / 400;
    jdnum += monthcount(month - 1, year);
    jdnum += day;
    return jdnum;
}

void date_to_ints(trans_date *date, int *year, int *month, int *day)
{
    int y=0, m=0, d=0;
    for(int i=0; i<4; i++)
    {
        y=y*10;
        y=y+(date->year[i] - '0');
    }
    for(int i=0; i<2; i++)
    {
        m=m*10;
        m=m+(date->month[i] - '0');
    }
    for(int i=0; i<2; i++)
    {
        d=d*10;
        d=d+(date->day[i] - '0');
    }
    *year = y;
    *month = m;
    *day = d;
}


int days_between(trans_date *start_date, trans_date *end_date)
{
    int year = 0, month = 0, day = 0;
    date_to_ints(start_date, &year, &month, &day);
    int start = makejd(year, month, day);
    date_to_ints(end_date, &year, &month, &day);
    int end = makejd(year, month, day);
    return end-start;
}

stock *get_ticker(stock **stocklist, char *ticker)
{
    int diff = 0;
    stock *local = *stocklist;
    if(!local)
    {
        local = calloc(1, sizeof(stock));
        local->ticker = strdup(ticker);
        *stocklist = local;
        return local;
    }
    else
    {
        diff = strcmp(ticker, local->ticker);
        if(diff == 0)
        {
            return local;
        }
        if(diff < 0)
        {
            local = calloc(1, sizeof(stock));
            local->ticker = strdup(ticker);
            local->next = *stocklist;
            *stocklist = local;
            return local;
        }
        return get_ticker(&(local->next), ticker);

    }
    // should be unreachable
    return NULL;
}

void add_buy(bought_units **buylist, transaction_node *node)
{
    if(*buylist)
    {
        add_buy(&((*buylist)->next), node);
    }
    else
    {
        bought_units *local_units = calloc(1, sizeof(bought_units));
        local_units->bought_price = node->price;
        local_units->original_units = node->units;
        local_units->held_units = node->units; // This tracks what remains if we sell
        local_units->buy_date = node->date; 
        *buylist = local_units;
    }
}

void add_bought_to_tail(bought_units **list, bought_units *node)
{
    if(*list)
        add_bought_to_tail(&((*list)->next), node);
    else
       *list = node;
}
void add_sold_to_tail(sold_units **list, sold_units *node)
{
    if(*list)
        add_sold_to_tail(&((*list)->next), node);
    else
       *list = node;
}
void process_sale(stock *stocklist, transaction_node *node)
{
    long double units_to_sell = node->units;
    long double units_this_transaction = 0.0;
    stock *localstock = stocklist;
    while(units_to_sell > 0.0)
    {
        if(!localstock->part_sold_units)
        {
            if(localstock->owned_units)
            {
                localstock->part_sold_units = localstock->owned_units;
                localstock->owned_units = localstock->owned_units->next;
                localstock->part_sold_units->next = NULL;
            }
            else
            {
                printf("Warning - [%s][%.8Lf] Unaccounted for - possible rounding errors\n", node->ticker, units_to_sell);
                // I think we will add this as a special transaction afterwards
                break;
            }
        }
        if(units_to_sell >= localstock->part_sold_units->held_units)
        {
            units_this_transaction = localstock->part_sold_units->held_units;
        }
        else if(units_to_sell < localstock->part_sold_units->held_units)
        {
            units_this_transaction = units_to_sell;
        }

        // Call another method for the actual creeation of the nodes
        // TO DO TO DO TODO TODO TODO
        units_to_sell -= units_this_transaction;

        localstock->part_sold_units->held_units -= units_this_transaction;

        sold_units *current_sale = calloc(1, sizeof(sold_units));
        current_sale->sold_price = node->price;
        current_sale->units_sold = units_this_transaction;
        current_sale->sale_date = node->date;

        add_sold_to_tail(&(localstock->part_sold_units->sales), current_sale);
        if(localstock->part_sold_units->held_units ==0)
        {
            add_bought_to_tail(&(localstock->sold_units), localstock->part_sold_units);

            localstock->part_sold_units = NULL;
        }

        
    }
}

// TODO:  CLEAR BUYS/TICKER

// THIS WILL ONLY BE CALLED ON BUY
void add_stock(stock **stocklist, transaction_node *node)
{
   stock* curr_stock = get_ticker(stocklist, node->ticker); 
   add_buy(&(curr_stock->owned_units), node);
}

void sell_stock(stock **stocklist, transaction_node *node)
{
    stock* curr_stock = get_ticker(stocklist, node->ticker);
    process_sale(curr_stock, node);
}

// This will evolve
void print_stock(stock *stocklist)
{
    stock *localstock= stocklist;
    while(localstock)
    {
        printf("Ticker [%s]:\n", localstock->ticker);
        printf("++++++ UNITS SOLD..... +++++++\n");
        bought_units *buys = localstock->sold_units;
        while(buys)
        {
            printf("\t[%s] Total: %.8Lf Remaining: %.8Lf Buy Price: %.8Lf\n", &(buys->buy_date), buys->original_units,
                    buys->held_units,  buys->bought_price);
            sold_units *sales = buys->sales;
            while(sales)
            {
                printf("\t\t[%s] Sold: %.8Lf Sell Price %.8Lf\n", &(sales->sale_date), sales->units_sold, sales->sold_price);
                sales = sales->next;
            }
            buys = buys->next;
        }

        printf("++++++ UNITS PART SOLD..... +++++++\n");
        buys = localstock->part_sold_units;
        while(buys)
        {
            printf("\t[%s] Total: %.8Lf Remaining: %.8Lf Buy Price: %.8Lf\n", &(buys->buy_date), buys->original_units,
                    buys->held_units,  buys->bought_price);
            sold_units *sales = buys->sales;
            while(sales)
            {
                printf("\t\t[%s] Sold: %.8Lf Sell Price %.8Lf\n", &(sales->sale_date), sales->units_sold, sales->sold_price);
                sales = sales->next;
            }
            buys = buys->next;
        }

        printf("++++++ UNITS HELD..... +++++++\n");
        buys = localstock->owned_units;
        while(buys)
        {
            printf("\t[%s] Total: %.8Lf Remaining: %.8Lf Buy Price: %.8Lf\n", &(buys->buy_date), buys->original_units,
                    buys->held_units,  buys->bought_price);
            buys = buys->next;
        }
        localstock = localstock->next;
    }
}

void populate_today(trans_date *date)
{
    time_t now = time(NULL);
    struct tm nowloc = *localtime(&now);
    int month = nowloc.tm_mon+1;
    int year = nowloc.tm_year+1900;
    date->day[1] = (nowloc.tm_mday %10) + '0';
    date->day[0] = ((nowloc.tm_mday/10) %10) + '0';
    date->month[1] = (month %10) + '0';
    date->month[0] = ((month/10) %10) + '0';
    date->year[3] = (year %10) + '0';
    date->year[2] = ((year/10) %10) + '0';
    date->year[1] = ((year/100) %10) + '0';
    date->year[0] = ((year/1000) %10) + '0';
    printf( "TODAY: %s\n", date);
}

void add_financial_year(financial_year **current, char *year, long double amount, int num_days)
{
    financial_year *local = *current;
    if(local)
    {
        int comp_years = strncmp(local->year, year, 4);
        if(0 == comp_years) 
        {
            if(amount>0)
            {
                //local->gains+=amount;
                // Not sure if this should be gte or gt 
                if(num_days > 365) local->discount_eligible_gains+=amount;
                else local->gains+=amount;
            }
            else
            {
                local->losses += amount;
            }
            return;

        }
        else if(comp_years < 0)
        {
            add_financial_year(&(local->next), year, amount, num_days);
            return;
        }
    }
    // This is sort of bad practice, but it avoids duplicating code.
    // If we get here we didn't find the year we're looking for
    local = calloc(1, sizeof(financial_year));
    local->next = *current;
    printf("Adding Financial Year [%s]\n", year);
    memcpy(local->year, year, 4);
    *current = local;
    // We're lazy, let's just make another function call
    add_financial_year(current, year, amount, num_days);

}

void clear_financial_years(financial_year **current)
{
    if(*current)
    {
        clear_financial_years(&((*current)->next));
        free(*current);
        *current=  NULL;
    }
}

void calculate_fin_year(trans_date *date, char *year)
{
    memcpy(year, date->year, 4);
    int num_yr = atoi(year);
    int month = (date->month[0] - '0') * 10 + (date->month[1] - '0');
    if(month>=7)
    {
        num_yr++;
    }
    sprintf(year, "%d", num_yr);
}

// Ticker, Buy Date, Buy Quantity, Buy Price, buy_remaining, Sell Date, Sell Q,  Sell Price, Bought Total , Sold Total, Profit/Loss, Days Held
void save_output(stock *stocklist)
{
    stock *localstock = stocklist;
    char *ticker;
    trans_date *buy_date, *sell_date;
    trans_date today = {0x00};
    long double buy_quant, buy_price, buy_remaining, sell_quant, sell_price, bought_total, sold_total, gain_loss; 
    long double original_price;

    int days_held;
    char filename[64];
    sprintf(filename, "output_%lu.csv", (unsigned long)time(NULL));
    FILE *fp = NULL;

    financial_year *fin_years = NULL;
    char current_fin_year[5] = {0x0};

    populate_today(&today);
    fp = fopen(filename, "w");
    if(fp)
    {
        fprintf(fp, "Ticker, Buy Date, Units Bought, Buy Price, Units Remaining, Sell Date, Units Sold, Sell Price, Buy"
                " Cost, Buy Cost adjusted,Sell Cost, Profit/Loss, Days Held\n");
        while(localstock)
        {
            ticker = localstock->ticker;
            bought_units *buys = NULL;
            for(int i=0; i<3; i++)
            {
                if(i==0) buys = localstock->sold_units;
                if(i==1) buys = localstock->part_sold_units;
                if(i==2) buys = localstock->owned_units;
                while(buys)
                {
                    buy_date = &(buys->buy_date);
                    buy_quant = buys->original_units;
                    bought_total = buy_quant* buy_price;
                    bought_total = buys->total;
                    buy_remaining = buys->held_units;
                    sold_units *sales = buys->sales;

                    sell_date = &today;
                    sell_quant = 0l;
                    sell_price = 0l;
                    sold_total = 0l;
                    gain_loss = 0l;
                    original_price = 0l;
                    days_held = days_between(buy_date, sell_date);
                    if(sales)
                    {
                        while(sales)
                        {
                            sell_date = &(sales->sale_date);
                            sell_quant = sales->units_sold;
                            sell_price = sales->sold_price;
                            sold_total = sell_quant *sell_price;
                            original_price = sell_quant * buy_price;
                            gain_loss = sold_total- original_price;
                            //original_price = original_price;
                            days_held = days_between(buy_date, sell_date);

                           calculate_fin_year(sell_date, current_fin_year); 
                           add_financial_year( &fin_years, current_fin_year, gain_loss, days_held);
                            if(buy_quant)
                            {
                                fprintf(fp, "%s,%s,%.8Lf,%.8Lf,%.8Lf,%s,%.8Lf,%.8Lf,%.8Lf,%.8Lf,%.8Lf,%.8Lf,%d\n",ticker,
                                        buy_date,buy_quant,buy_price,buy_remaining, sell_date, sell_quant,sell_price,
                                        bought_total, original_price, sold_total, gain_loss,
                                        days_held );
                                buy_quant = 0;
                                buy_remaining = 0;
                            }
                            else
                            {
                                fprintf(fp, "%s,%s,,%.8Lf,,%s,%.8LF,%.8Lf,%.8LF,%.8Lf,%.8Lf,%.8Lf,%d\n",ticker,
                                        buy_date,buy_price, sell_date, sell_quant,sell_price, bought_total,
                                        original_price, sold_total, gain_loss,
                                        days_held );
                            }
                            sales = sales->next;
                        }
                    }
                    else
                    {
                        fprintf(fp, "%s,%s,%.8Lf,%.8Lf,%.8Lf,,,,%.8Lf,%.8Lf,,,%d\n",ticker,
                                buy_date,buy_quant,buy_price,buy_remaining,  bought_total, buy_price*buy_remaining, 
                                days_held );
                    }
                    buys = buys->next;
                }
            }
            localstock=localstock->next;
        }

        if(fin_years)
        {
            fprintf(fp, "\n\nFinancial year, Ineligible Gain, Discount Eligible Gain, Total Gain,Total Loss,,"
                    "Net ineligible, Net eligible,,Net Total\n");
            
            financial_year*curr_yr = fin_years;
            long double net_gain =0.0, total_gain = 0.0, total_loss = 0.0, tmp_loss = 0.0, tmp = 0.0;
            long double gain = 0.0, discount_eligible_gain = 0.0, tmp_gain = 0.0, tmp_discount_eligible_gain = 0.0;
            while(curr_yr)
            {
                total_loss = curr_yr->losses;
                tmp_loss = total_loss;
                gain = curr_yr->gains;
                discount_eligible_gain = curr_yr->discount_eligible_gains;
                total_gain = gain + discount_eligible_gain;

                tmp = gain + total_loss;
                // If the loss totally takes away the gain, then apply it to discounted gain
                if(tmp < 0.0)
                {
                    tmp_gain = 0.0;
                    //tmp_loss = total_loss - gain;
                    tmp_discount_eligible_gain = discount_eligible_gain +tmp;
                }
                // else don't
                else
                {
                    tmp_gain = tmp;
                    tmp_discount_eligible_gain = discount_eligible_gain;
                }

                // if we have any eligible discount availble, apply it
                if(tmp_discount_eligible_gain > 0)
                {
                    net_gain = tmp_gain + tmp_discount_eligible_gain /2.0;
                }
                else
                {
                    net_gain = tmp_gain + tmp_discount_eligible_gain;
                }

                fprintf(fp, "%.4s, %Lf,%Lf,%Lf,%Lf,,%Lf,%Lf,,%Lf\n", curr_yr->year, gain,
                        discount_eligible_gain, total_gain, total_loss, tmp_gain, tmp_discount_eligible_gain,
                        net_gain);
                curr_yr = curr_yr->next;
            }
        fprintf(fp, "\n\n\"DISCLAIMER:  Double check all values and make sure you understand what is what\"\n");
        fprintf(fp, "\"\t\tSoftware author takes no responsibility for accuracy of calculations, nor how they are used"
        "on a tax return\"\n");
        fprintf(fp, "\n\"Recommend using this to create entries in MyTax's CGT tracking tool. \"\n");
        fprintf(fp, "\"If a year has both ineligible and eligible transactions, declare them separately\"\n");

        fprintf(fp, "\n\n\"** NO RESPONSIBILITY IS TAKEN FOR THE CALCULATION NOR HOW THE DATA IS USED\"\n");
        fprintf(fp, "\"\tTHE AUTHOR IS *NOT* A QUALIFIED TAX ACCOUNTANT AND THEIR INTERPRETATION OF HOW\"\n");
        fprintf(fp, "\"\tCGT IS CALCULATED SHOULD NOT BE ASSUMED TO BE CORRECT.  UNDERSTANDING HOW THE DATA\"\n");
        fprintf(fp, "\"\tIS GENERATED AND WHAT IT MEANS IS IMPORTANT IF USING THIS DATA TO POPULATE YOUR TAX\"\n");
        fprintf(fp,"\"\tRETURN - IF IN DOUBT, ENGAGE A PROFESSIONAL.\"\n");
        fprintf(fp,"\n\"THIS TOOL WAS ORIGINALLY WRITTEN AS A BASIC ESTIMATE OF CGT - IT DOES NOT CLAIM\"\n");
        fprintf(fp,"\"\tTO BE CORRECT OR RELIABLE - ESPECIALLY IN COMPLEX SITUATIONS.  UNDERSTANDING HOW CGT\"\n");
        fprintf(fp, "\"\tSHOULD BE CALCULATED IS STILL NECESSARY TO USE THE VALUES IN THIS OUTPUT\"\n");
        }

        fclose(fp);
    }

    clear_financial_years(&fin_years);
}

void clear_sells(sold_units **sold)
{
    if(*sold)
    {
        clear_sells(&((*sold)->next));
        free(*sold);
        *sold = NULL;
    }
}
void clear_buys(bought_units **buys)
{
    if(*buys)
    {
        clear_buys(&((*buys)->next));
        clear_sells(&((*buys)->sales));
        free(*buys);
        *buys = NULL;
    }
}

void clear_stocks(stock **stocklist)
{
    if(*stocklist)
    {
        clear_stocks(&((*stocklist)->next));
        clear_buys(&((*stocklist)->owned_units));
        clear_buys(&((*stocklist)->sold_units));
        clear_buys(&((*stocklist)->part_sold_units));
        free((*stocklist)->ticker);
    }

}

   

#define MAX_LINE_LEN (1024)
int main(int argn, char*args[])
{

    // BPossibly used unsafely, no bounds checking
    char line[MAX_LINE_LEN] = {0x00};
    FILE *inFile = NULL;

   queued_transaction buys = {0x00};
    queued_transaction sells = {0x00};

    gargs = args;

    if(argn < 2)
    {
        usage("No Filename spaecified");
    }
    // OPEN FILE
    inFile = fopen(args[1], "r");
    if(!inFile)
    {
        usage("COuldn't find specified file");
    }

    // WHILE READ LINE
    while(NULL != fgets(line,  MAX_LINE_LEN, inFile))
    {
        if(parse_line(line, &buys, &sells))
        {
            continue;
        }

    }
//    printf("+++++++++++++++++++++ BUYS ++++++++++++++++++++++++++++++\n");
//    print_queue(&buys);

//  printf("+++++++++++++++++++++ SELLS ++++++++++++++++++++++++++++++\n");
//  print_queue(&sells);

    //XXXX
    stock *my_stock = NULL; 
    transaction_node *curr_node = buys.head;
    while(curr_node)
    {
        add_stock(&my_stock, curr_node);
        curr_node = curr_node->next;
    }

    curr_node = sells.head;
    while(curr_node)
    {
        sell_stock(&my_stock, curr_node);
        curr_node = curr_node->next;
    }

    //print_stock(my_stock);
    save_output(my_stock);
    
    clear_stocks(&my_stock);
    clear_queue(&buys);
    clear_queue(&sells);
}
