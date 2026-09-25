def largestRectangleArea_step_by_step(heights):
    max_area = 0
    stack = []

    print("Istogramma:", heights)
    print()

    for i in range(len(heights)):
        start = i

        print(f"--- PASSO {i} ---")
        print(f"Barra corrente: indice={i}, altezza={heights[i]}")
        print("Stack prima:", stack)

        while stack and stack[-1][1] > heights[i]:

            index, height = stack.pop()

            width = i - index
            area = height * width

            print()
            print(f"Rimuovo dalla stack: ({index}, {height})")
            print(f"Larghezza = {i} - {index} = {width}")
            print(f"Area = {height} * {width} = {area}")

            max_area = max(max_area, area)

            print("Max area:", max_area)

            start = index

            print(f"Il nuovo start diventa {start}")

        stack.append((start, heights[i]))

        print()
        print(f"Inserisco ({start}, {heights[i]})")
        print("Stack dopo:", stack)
        print()

    print("--- FINE SCANSIONE ---")
    print("Elementi rimasti nella stack:", stack)
    print()

    for index, height in stack:

        width = len(heights) - index
        area = height * width

        print(f"Controllo ({index}, {height})")
        print(f"Larghezza = {len(heights)} - {index} = {width}")
        print(f"Area = {height} * {width} = {area}")

        max_area = max(max_area, area)

        print("Max area:", max_area)
        print()

    print("AREA MASSIMA FINALE:", max_area)

    return max_area


heights = [2, 1, 5, 6, 2, 3]

largestRectangleArea_step_by_step(heights)
