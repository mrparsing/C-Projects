#include <stdio.h>
#include <stdlib.h>

float convert_C_to_F(float temperature)
{
    return temperature * 9 / 5 + 32;
}

float convert_C_to_K(float temperature)
{
    return temperature + 273.15;
}

float convert_K_to_F(float temperature)
{
    return (temperature - 273.15) * 9 / 5 + 32;
}

float convert_K_to_C(float temperature)
{
    return temperature - 273.15;
}

float convert_F_to_C(float temperature)
{
    return (temperature - 32) * 5 / 9;
}

float convert_F_to_K(float temperature)
{
    return (temperature - 32) * 5 / 9 + 273.15;
}

void convert(float temperature, char unit_from, char unit_to)
{
    switch (unit_from)
    {
    case 'C':
        switch (unit_to)
        {
        case 'F':
            printf("%.2f °C = %.2f °F\n", temperature, convert_C_to_F(temperature));
            break;
        case 'K':
            printf("%.2f °C = %.2f K\n", temperature, convert_C_to_K(temperature));
            break;
        default:
            printf("Unsupported conversion.\n");
            break;
        }
        break;

    case 'K':
        switch (unit_to)
        {
        case 'F':
            printf("%.2f K = %.2f °F\n", temperature, convert_K_to_F(temperature));
            break;
        case 'C':
            printf("%.2f K = %.2f °C\n", temperature, convert_K_to_C(temperature));
            break;
        default:
            printf("Unsupported conversion.\n");
            break;
        }
        break;

    case 'F':
        switch (unit_to)
        {
        case 'C':
            printf("%.2f °F = %.2f °C\n", temperature, convert_F_to_C(temperature));
            break;
        case 'K':
            printf("%.2f °F = %.2f K\n", temperature, convert_F_to_K(temperature));
            break;
        default:
            printf("Unsupported conversion.\n");
            break;
        }
        break;

    default:
        printf("Invalid source unit.\n");
        break;
    }
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: %s <temperature> <source unit> <target unit>\n", argv[0]);
        printf("Units: C = Celsius, F = Fahrenheit, K = Kelvin\n");
        return 1;
    }

    float temperature = atof(argv[1]);
    char unit_from = argv[2][0];
    char unit_to = argv[3][0];

    convert(temperature, unit_from, unit_to);

    return 0;
}