#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// define the default financial characteristics. These will be used if no input files are provided.
#define FL_DEFAULT_DEBT_PCT  0.03
#define NFL_DEFAULT_DEBT_PCT 0.03
#define FL_DEFAULT_HOUSE_PRICE 175000
#define NFL_DEFAULT_HOUSE_PRICE 175000
#define FL_DEFAULT_INITIAL_SAVINGS 5000
#define NFL_DEFAULT_INITIAL_SAVINGS 5000
#define FL_DEFAULT_INITIAL_CHECKING 0
#define NFL_DEFAULT_INITIAL_CHECKING 0
#define FL_DEFAULT_INITIAL_DEBT  30100 
#define NFL_DEFAULT_INITIAL_DEBT  30100 
#define NFL_DEFAULT_ADDPAY_AMT 1
#define FL_DEFAULT_ADDPAY_AMT 15
#define NFL_DEFAULT_SALARY 59000
#define FL_DEFAULT_SALARY 59000 
#define NFL_DEFAULT_LOAN_RT 0.05
#define FL_DEFAULT_LOAN_RT 0.045
#define NFL_DEFAULT_SAVINGS_RT 0.01
#define FL_DEFAULT_SAVINGS_RT 0.07
#define NFL_DEFAULT_DEBT_RT 0.2
#define FL_DEFAULT_DEBT_RT 0.2
#define NFL_DEFAULT_DOWNPAY_PCT 0.05
#define FL_DEFAULT_DOWNPAY_PCT 0.2
#define NFL_DEFAULT_CHECKING_PCT 0.3
#define FL_DEFAULT_CHECKING_PCT 0.3
#define NFL_DEFAULT_SAVING_PCT 0.2
#define FL_DEFAULT_SAVING_PCT 0.2
#define FL_DEFAULT_RENT_AMT 850
#define NFL_DEFAULT_RENT_AMT 850


#define SIMULATION_LENGTH_YEARS 41
#define DEFAULT_MORTGAGE_TERM 30

struct financialIdentity 
{
	bool has_loan;
	double savings;
	double checking;
	double debt;
	double loan;
	int yearsWithDebt;
	int yearsRented;
	double debtPaid;
	double initial_debt;
	double initial_savings;
	double initial_checking;
	double initial_wealth;
	double house_price;
	double rent_amt;
	double loan_int_rate;
	double savings_int_rate;
	double debt_int_rate;
	double salary;
	double debt_pct;
	double downpay_pct;
	double addl_pay_amt;
	double savings_pct;
	double checking_pct;
	double monthlyPayment;
	double total_loanInterest;
	double total_savingsInterest;
	double total_debtInterest;
};

// struct with financial qualities that will be read from input file
struct parameterizedValues 
{
	double initial_debt;
	double initial_savings;
	double initial_checking;
	double house_price;
	double rent_amt;
	double loan_int_rate;
	double savings_int_rate;
	double debt_int_rate;
	double salary;
	double debt_pct;
	double downpay_pct;
	double addl_pay_amt;
	double savings_pct;
	double checking_pct;
};

// reflects savings account after one year
void savingsPlacement(struct financialIdentity* person, double interestRate) 
{
	person->savings = (person->savings) * (interestRate + 1); // interest added to savings account
	person->total_savingsInterest += person->savings * interestRate; // yearly interest is added to total earned
}

//calculates amount of debt and money left in accounts after 1 year
void debt(struct financialIdentity* person, double interestRate, double addlPay) 
{
	double checking_pmt, savings_pmt, total_pmt ;

	for (int i = 0; i < 12; i++) 
	{
		if (person->debt > 0)
		{
			total_pmt = checking_pmt = savings_pmt = 0;

			total_pmt = (person->debt * person->debt_pct) + person->addl_pay_amt;
			if (total_pmt > person->debt)
			{
				total_pmt = person->debt;
			}


			if (person->checking < total_pmt)
			{
				checking_pmt = person->checking;
				savings_pmt = total_pmt - checking_pmt;

			}
			else
			{
				checking_pmt = total_pmt;
			}

			person->checking -= checking_pmt;
			person->savings -= savings_pmt;

			person->debtPaid += total_pmt;
			person->debt -= total_pmt;
					
		}
	}

	person->debt = person->debt * (1 + interestRate); // debt compounded annually
	person->total_debtInterest += person->debt * interestRate; // yearly interest added to total interest paid on debt
	person->yearsWithDebt++; // years with debt incremented

}

// calculates money left in accounts after 1 year of renting
void rent(struct financialIdentity *person, double rentAmt) 
{
	double checking_pmt, savings_pmt;

	for (int i = 0; i < 12; i++) 
	{

		checking_pmt = savings_pmt = 0;

		// if checking account cannot cover monthly rent, difference is payed from savings
		if (person->checking < rentAmt)
		{
			checking_pmt = person->checking;
			savings_pmt = rentAmt - checking_pmt;

		}
		else
		{
			checking_pmt = rentAmt;
		}

		person->checking -= checking_pmt;
		person->savings -= savings_pmt;
	}

	person->yearsRented++; // years rented incremented
}

// represents one year of having a house
void house(struct financialIdentity* person, 
double mortgageAmount, double interestRate, double mortgageTerm) 
{
	double checking_pmt, savings_pmt, total_pmt ;

	// if monthly payment hasn't been calculated, calculate
	if (person->monthlyPayment == 0)
	{
		double i = interestRate / 12;
		double discountFactor = ((pow(1 + i, (12 * mortgageTerm))) - 1) / (i * (pow(1 + i, (12 * mortgageTerm))));
		person->monthlyPayment = mortgageAmount / discountFactor;
	}

	for (int x = 0; x < 12; x++) 
	{
		person->total_loanInterest += (person->loan) * interestRate / 12; // monthly interest added to total interest paid on loan
		person->loan += (person->loan) * interestRate / 12 ; // interest added back into loan balance each month

		total_pmt = checking_pmt = savings_pmt = 0; 

		// if monthly payment is greater than loan balance,
		// pay off rest of loan balance but nothing more.
		total_pmt = person->monthlyPayment;
		if (total_pmt > person->loan)
		{

			total_pmt = person->loan;
		}

		// if checking account cannot cover payment, difference payed from savings
		if (person->checking < total_pmt) 
		{
			checking_pmt = person->checking;
			savings_pmt = total_pmt - checking_pmt;

		}
		else 
		{
			checking_pmt = total_pmt;
		}

		person->checking -= checking_pmt;
		person->savings -= savings_pmt;
		person->loan -= total_pmt;

	}	
}

// simulate a person's lifetime wealth
int * simulate(struct financialIdentity* person, double yearlySalary, bool financiallyLiterate) 
{
	int* wealthArray;
	
	wealthArray = (int*)malloc(SIMULATION_LENGTH_YEARS * sizeof(int));

	wealthArray[0] = person->initial_wealth;
	double downpayment_amt, original_loan_amt = 0;

	downpayment_amt = person->downpay_pct * person->house_price;

	for (int i = 1; i < SIMULATION_LENGTH_YEARS; i++) 
	{
		// portion of yearly salary deposited into accounts
		person->checking += (yearlySalary * person->checking_pct);
		person->savings += (yearlySalary * person->savings_pct);

		// if person can afford down payment and does not already have a house loan, take out a loan
		if ((person->savings > downpayment_amt) && (person->has_loan == false))
		{
				person->has_loan = true;
				person->savings -= downpayment_amt;
				original_loan_amt =  person->loan = person->house_price - downpayment_amt;			
		}

		if (person->has_loan)
		{
			// if person has not paid off their loan, call house()
			if (person->loan > 0)
			{
				house(person, original_loan_amt, person->loan_int_rate, DEFAULT_MORTGAGE_TERM);
			}
		}

		// if person does not have a house loan, undergo one year of rent
		else 
		{
			rent(person, person->rent_amt);
		}

		if (person->debt > 0)
		{
			debt(person, person->debt_int_rate, person->addl_pay_amt);
		}

		savingsPlacement(person, person->savings_int_rate);

		int wealth = person->checking + person->savings - person->loan - person->debt;
		wealthArray[i] = wealth; // each year's wealth added to array
	}

	return wealthArray;
}

// initialize financial identity using default values
int initialize(struct financialIdentity *person, bool financiallyLiterate)
{

	person->has_loan = false;
	person->loan = 0;
	person->debtPaid = 0;
	person->yearsWithDebt = 0;
	person->yearsRented = 0;

	if (financiallyLiterate)
	{
		person->debt_pct = FL_DEFAULT_DEBT_PCT;
		person->house_price = FL_DEFAULT_HOUSE_PRICE;
		person->savings = person->initial_savings = FL_DEFAULT_INITIAL_SAVINGS;
		person->checking = person->initial_checking = FL_DEFAULT_INITIAL_CHECKING;
		person->debt = person->initial_debt = FL_DEFAULT_INITIAL_DEBT;
		person->salary = FL_DEFAULT_SALARY;
		person->addl_pay_amt = FL_DEFAULT_ADDPAY_AMT;
		person->initial_wealth = person->checking + person->savings - person->debt;
		person->loan_int_rate = FL_DEFAULT_LOAN_RT;
		person->savings_int_rate = FL_DEFAULT_SAVINGS_RT;
		person->debt_int_rate = FL_DEFAULT_DEBT_RT;
		person->downpay_pct = FL_DEFAULT_DOWNPAY_PCT;
		person->savings_pct = FL_DEFAULT_SAVING_PCT;
		person->checking_pct = FL_DEFAULT_CHECKING_PCT;
		person->rent_amt = FL_DEFAULT_RENT_AMT;
	}
	else
	{
		person->debt_pct = NFL_DEFAULT_DEBT_PCT;
		person->house_price = NFL_DEFAULT_HOUSE_PRICE;
		person->savings = person->initial_savings = NFL_DEFAULT_INITIAL_SAVINGS;
		person->checking = person->initial_checking = NFL_DEFAULT_INITIAL_CHECKING;
		person->debt = person->initial_debt = NFL_DEFAULT_INITIAL_DEBT;
		person->salary = NFL_DEFAULT_SALARY;
		person->addl_pay_amt = NFL_DEFAULT_ADDPAY_AMT;
		person->initial_wealth = person->checking + person->savings - person->debt;
		person->loan_int_rate = NFL_DEFAULT_LOAN_RT;
		person->savings_int_rate = NFL_DEFAULT_SAVINGS_RT;
		person->debt_int_rate = NFL_DEFAULT_DEBT_RT;
		person->downpay_pct = NFL_DEFAULT_DOWNPAY_PCT;
		person->savings_pct = NFL_DEFAULT_SAVING_PCT;
		person->checking_pct = NFL_DEFAULT_CHECKING_PCT;
		person->rent_amt = NFL_DEFAULT_RENT_AMT;
	}
	person->total_loanInterest = 0;
	person->total_savingsInterest = 0;
	person->total_debtInterest = 0;
	person->monthlyPayment = 0;

	return 0;
}

// retrieve financial identity from input file
int getPersonInfo(char* fname, struct parameterizedValues *person) {

	FILE* inFile;
	int x;

	inFile = fopen(fname, "r");

	if (inFile == NULL) {
		printf("Could not read file.  Program will use defaults\n");
		return -1;
	}

	x = fscanf(inFile, "Initial Savings: %lf\n", &(person->initial_savings));
	x = fscanf(inFile, "Initial Checking: %lf\n", &(person->initial_checking));
	x = fscanf(inFile, "House Price: %lf\n", &(person->house_price));
	x = fscanf(inFile, "Salary: %lf\n", &(person->salary));
	x = fscanf(inFile, "Rent Amount: %lf\n", &(person->rent_amt));
	x = fscanf(inFile, "Loan Interest Rate: %lf\n", &(person->loan_int_rate));
	x = fscanf(inFile, "Savings Interest Rate: %lf\n", &(person->savings_int_rate));
	x = fscanf(inFile, "Debt Interest Rate: %lf\n", &(person->debt_int_rate));
	x = fscanf(inFile, "Initial Debt: %lf\n", &(person->initial_debt));
	x = fscanf(inFile, "Debt Percent: %lf\n", &(person->debt_pct));
	x = fscanf(inFile, "Down Payment Percent: %lf\n", &(person->downpay_pct));
	x = fscanf(inFile, "Additional Payment Amount: %lf\n", &(person->addl_pay_amt));
	x = fscanf(inFile, "Savings Percent: %lf\n", &(person->savings_pct));
	x = fscanf(inFile, "Checking Percent: %lf\n", &(person->checking_pct));

	fclose(inFile);
	return 0;
}

main(int argc, char* argv[])
{
	struct financialIdentity nfl, fl;
	struct parameterizedValues fl_overRide, nfl_overRide;
	char fflname[255], fnflname[255];
	int rc;
	int *fl_wealthArray, *nfl_wealthArray;

	strcpy(fflname, "");
	strcpy(fnflname, "");

	// initialize 2 people with default values
	initialize(&fl, true);
	initialize(&nfl, false);

	// if no input files provided, use defaults
	if (argc < 3)
	{
		//use defaults
	}
	else
	{
		strcpy(fflname, argv[1]);
		rc = getPersonInfo(fflname, &fl_overRide); // get financially literate info from input file

		// construct identity of financially literate person with values from input
		if (rc == 0)
		{
			fl.initial_debt = fl_overRide.initial_debt;
			fl.initial_savings = fl_overRide.initial_savings;
			fl.initial_checking = fl_overRide.initial_checking;
			fl.house_price = fl_overRide.house_price;
			fl.salary = fl_overRide.salary;
			fl.rent_amt = fl_overRide.rent_amt;
			fl.loan_int_rate = fl_overRide.loan_int_rate;
			fl.savings_int_rate = fl_overRide.savings_int_rate;
			fl.debt_int_rate = fl_overRide.debt_int_rate;
			fl.debt_pct = fl_overRide.debt_pct;
			fl.downpay_pct = fl_overRide.downpay_pct;
			fl.addl_pay_amt = fl_overRide.addl_pay_amt;
			fl.savings_pct = fl_overRide.savings_pct;
			fl.checking_pct = fl_overRide.checking_pct;
		}

		strcpy(fnflname, argv[2]);
		rc = getPersonInfo(fnflname, &nfl_overRide); // get non-financially literate info from input file

		// construct identity of non-financially literate person with values from input
		if (rc == 0)
		{
			nfl.initial_debt = nfl_overRide.initial_debt;
			nfl.initial_savings = nfl_overRide.initial_savings;
			nfl.initial_checking = nfl_overRide.initial_checking;
			nfl.house_price = nfl_overRide.house_price;
			nfl.salary = nfl_overRide.salary;
			nfl.rent_amt = nfl_overRide.rent_amt;
			nfl.loan_int_rate = nfl_overRide.loan_int_rate;
			nfl.savings_int_rate = nfl_overRide.savings_int_rate;
			nfl.debt_int_rate = nfl_overRide.debt_int_rate;
			nfl.debt_pct = nfl_overRide.debt_pct;
			nfl.downpay_pct = nfl_overRide.downpay_pct;
			nfl.addl_pay_amt = nfl_overRide.addl_pay_amt;
			nfl.savings_pct = nfl_overRide.savings_pct;
			nfl.checking_pct = nfl_overRide.checking_pct;
		}
	}

	// simulate wealth of fl and nfl
	fl_wealthArray = simulate(&fl, fl.salary, true);
	nfl_wealthArray = simulate(&nfl, nfl.salary, true);

	// write both wealth arrays to output files
	FILE *flOutfile;

	flOutfile = fopen("flArray.txt", "w");
	if (flOutfile == NULL) {
		printf("Could not open output file.");
	}
	else
	{
		for (int i = 0; i < SIMULATION_LENGTH_YEARS ; i++) {
			fprintf(flOutfile, "%d\n", (int)fl_wealthArray[i]);
		}
		fclose(flOutfile);
	}

	FILE *nflOutfile;

	nflOutfile = fopen("nflArray.txt", "w");
	if (nflOutfile == NULL) {
		printf("Could not open output file.");
	}
	else
	{
		for (int i = 0; i < SIMULATION_LENGTH_YEARS; i++) {
			fprintf(nflOutfile, "%d\n", (int)nfl_wealthArray[i]);
		}
		fclose(nflOutfile);
	}

	free(nfl_wealthArray);
	free(fl_wealthArray);
}

