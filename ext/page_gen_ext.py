from random import randint, random

def generate_pages(file_name, n, max_page, write_ratio, prev_prob, persistent_ratio):
    seen_pages = {}
    with open(file_name, 'w') as file:
        last = randint(1, max_page)
        for i in range(n):
            if(random() > prev_prob):
                page = randint(1, max_page)
            else:
                page = last
            if(random() > write_ratio):
                op = 'r'
            else:
                op = 'w'

            persistent = seen_pages.get(page)

            if(persistent == None):
                if(random() > persistent_ratio):
                    persistent = "p"
                else:
                    persistent = "np"
            
                seen_pages[page] = persistent

            last = page

            file.write(str(page) + ' ' + op + ' ' + persistent + '\n')



print("Input File Name: ")
file_name = input()
print("Input Number of Page Accesses: ")
n = int(input())
print("Input Maximum Page Number: ")
max_page = int(input())
print("Input Write Ratio: ")
write_ratio = float(input())
print("Input Probability of Accessing the Previous Page: ")
prev_prob = float(input())
print("Input Probability of Page Being Persistent: ")
persistent_ratio = float(input())

generate_pages(file_name, n, max_page, write_ratio, prev_prob, persistent_ratio)