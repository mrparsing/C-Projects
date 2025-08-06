# Binary Search

This program implements the binary search algorithm to find a number in a sorted array of integers. The user inputs a number to search for, and the program returns the index of that number in the array if it exists, otherwise it informs the user that the number was not found.

## How the Code Works

1. **Sorted Array**: The program uses a sorted integer array:
   ```c
   int array[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};

2.	User Input: It prompts the user to enter a number to search.
3.	Binary Search Function: The binary_search function takes as input:
- the sorted array,
- the size of the array,
- the value to search for (target).
The function uses two indexes, left and right, to define the current search boundaries.
4.	Binary Search Algorithm:
- Calculates the middle index between left and right.
- If the element at middle equals the target, it returns middle.
- If the element at middle is less than the target, it moves left to middle + 1 (search right half).
- If the element at middle is greater than the target, it moves right to middle - 1 (search left half).
- This process repeats while left is less than or equal to right.
- If the element is not found, it returns -1.
5.	Output: Prints the index if the number is found; otherwise prints a “not found” message.

What is Binary Search?

Binary search is an efficient algorithm to find an element in a sorted array, with time complexity O(log n). It works by repeatedly dividing the search interval in half:
- Start by comparing the target to the middle element.
- If the target is greater, continue searching in the right half.
- If the target is smaller, continue searching in the left half.
- Repeat until the element is found or the search interval is empty.

This method is much faster than linear search, especially on large arrays.