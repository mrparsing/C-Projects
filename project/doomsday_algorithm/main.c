#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void calculate_date(int year, int month, int day);
bool leap_year(int year);
int calculate_doomsday_century(int century);
int calculate_doomsday_year(int year, int doomsday_century);
void wait_for_enter(void);
bool check_date(int year, int month, int day);

const char *days[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"};

const int anchor_date[] = {
    3, 28, 14, 4, 9, 6, 11, 8, 5, 10, 7, 12};

bool leap_year(int year)
{
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

int calculate_doomsday_century(int century)
{
    int day = century % 4;

    if (day == 0)
        return 2;
    else if (day == 1)
        return 0;
    else if (day == 2)
        return 5;
    else
        return 3;
}

int calculate_doomsday_year(int year, int doomsday_century)
{
    int last_digit = year % 100;
    int integer_part = last_digit / 12;
    int frac_part = last_digit % 12;
    int temp = frac_part / 4;

    return (integer_part + frac_part + temp + doomsday_century) % 7;
}

void calculate_date(int year, int month, int day)
{
    int doomsday_century = calculate_doomsday_century(year / 100);
    int doomsday_year = calculate_doomsday_year(year, doomsday_century);

    int anchor_day = anchor_date[month - 1];

    if (leap_year(year) && (month == 1 || month == 2))
        anchor_day += 1;

    int day_index = ((day - anchor_day + doomsday_year) % 7 + 7) % 7;

    printf("\n%s\n", days[day_index]);
}

void wait_for_enter(void)
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
    {
    }

    printf("Press Enter to show solution...");
    fflush(stdout);

    getchar();
}

bool check_date(int year, int month, int day)
{
    int months[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};

    if (year < 1)
        return false;

    if (month < 1 || month > 12)
        return false;

    if (month == 2 && leap_year(year))
        months[1] = 29;

    if (day < 1 || day > months[month - 1])
        return false;

    return true;
}

int main(void)
{
    int choice;
    int day;
    int month;
    int year;

    int months[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};

    srand((unsigned int)time(NULL));

    while (true)
    {
        printf("\nDoomsday Algorithm\n");
        printf("1) Generate a date\n");
        printf("2) Calculate a date\n");
        printf("0) Exit\n");

        if (scanf("%d", &choice) != 1)
        {
            int c;

            while ((c = getchar()) != '\n' && c != EOF)
            {
            }

            printf("Invalid choice\n");
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            year = rand() % 3000 + 1;
            month = rand() % 12 + 1;

            int max_day = months[month - 1];

            if (month == 2 && leap_year(year))
                max_day = 29;

            day = rand() % max_day + 1;

            printf("%d/%d/%d\n", day, month, year);

            wait_for_enter();
            calculate_date(year, month, day);

            break;
        }

        case 2:
            printf("Enter day: ");

            if (scanf("%d", &day) != 1)
            {
                printf("\nDate invalid...\n");

                while (getchar() != '\n')
                {
                }

                break;
            }

            printf("Enter month: ");

            if (scanf("%d", &month) != 1)
            {
                printf("\nDate invalid...\n");

                while (getchar() != '\n')
                {
                }

                break;
            }

            printf("Enter year: ");

            if (scanf("%d", &year) != 1)
            {
                printf("\nDate invalid...\n");

                while (getchar() != '\n')
                {
                }

                break;
            }

            if (check_date(year, month, day))
                calculate_date(year, month, day);
            else
                printf("\nDate invalid...\n");

            break;

        case 0:
            printf("Bye\n");
            return 0;

        default:
            printf("Invalid choice\n");
            break;
        }
    }
}