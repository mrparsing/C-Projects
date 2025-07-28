# Temperature Converter

A simple command-line C program to convert temperatures between Celsius (C), Fahrenheit (F), and Kelvin (K).

---

Usage
```
./converter <temperature> <source unit> <target unit>
```
Where:
- <temperature> is a number (can be decimal)
- <source unit> and <target unit> are one of C, F, or K

Example
```
./converter 100 C F
```
Output:
```
100.00 °C = 212.00 °F
```

---

Supported Conversions
- Celsius → Fahrenheit
- Celsius → Kelvin
- Fahrenheit → Celsius
- Fahrenheit → Kelvin
- Kelvin → Celsius
- Kelvin → Fahrenheit

If you try to convert to the same unit or an unsupported one, you’ll get an error message.

---

How It Works
- The program reads the temperature, source unit, and target unit from the command line.
- Each conversion is implemented as its own function (e.g., convert_C_to_F, convert_F_to_K).
- The main convert function routes the call to the correct conversion function.
- Invalid or unsupported units result in error messages.

---

Compilation
```
gcc converter.c -o converter
```