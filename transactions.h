#include <stdio.h>

#pragma once

// LIMITATIONS:
// Not certain how it handles out of order transaction - we process first come, first serve, so if a later date is added
// later it may impace calculations.  TODO LATER: 

typedef enum
{
    TT_UNKNOWN,
    TT_BUY,
    TT_SELL
} TRANSACTION_TYPE;


typedef struct sold_units_s sold_units;

// Store the date as a string YYYYMMDD - this is mainly to allow easy comaparison 
typedef struct
{
    char year[4];
    char month[2];
    char day[2];
}
trans_date;

struct sold_units_s
{
    long double sold_price;
    long double units_sold;
    sold_units *next;
    trans_date sale_date;
} ;

typedef struct bought_units_s bought_units;
struct bought_units_s
{
   long double bought_price; // Consider more accurate types if needed
   long double original_units;
   long double held_units;
   sold_units *sales;
   bought_units *next;
   trans_date buy_date;
};

typedef struct stock_s stock;
struct  stock_s
{
    char *ticker;
    //char *description; // What to do if this changes?
    bought_units *owned_units;
    bought_units *part_sold_units;
    bought_units *sold_units;
    stock *next;
};


typedef struct transaction_node transaction_node;
struct transaction_node
{
    trans_date date;
    char *ticker;
    long double units;
    long double price;
    long double total; // This is just a sanity test - I think
    transaction_node *next;
} ;

typedef struct queued_transaction queued_transaction;
struct queued_transaction
{
    transaction_node *head;
    transaction_node *tail;
    trans_date latest_date;
};

typedef struct
{
    long double units_to_sell;
    long double units_sold;
    // do we need anything off the buy transaction?
} transaction;

typedef struct financial_year financial_year;

struct financial_year
{
    char year[4];
    long double gains;
    long double discount_eligible_gains;
    long double losses;
    financial_year *next;
};
