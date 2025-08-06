#include <stdio.h>

int binary_search(int arr[], int size, int target) {
    int left = 0;
    int right = size - 1;

    while (left <= right) {
        int middle = left + (right - left) / 2;

        if (arr[middle] == target)
            return middle;
        else if (arr[middle] < target)
            left = middle + 1;
        else
            right = middle - 1;
    }

    return -1;
}

int main() {
    int array[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    int size = sizeof(array) / sizeof(array[0]);

    int number_to_find;
    printf("Enter the number to search for: ");
    scanf("%d", &number_to_find);

    int result = binary_search(array, size, number_to_find);

    if (result != -1)
        printf("Number found at index %d.\n", result);
    else
        printf("Number not found in the array.\n");

    return 0;
}
