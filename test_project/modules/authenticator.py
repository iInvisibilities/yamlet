try:
    file = open("/dev/shm/yamlet_shared_buffer", "r")
except FileNotFoundError:
    print("FROM PYTHON: Shared buffer not found.")

'''for line in file:
    print("FROM PYTHON SHARED BUFFER:", line.strip())
file.close()
'''
print("FROM PYTHON: MODULE RUNNING")
print("FROM PYTHON: listening on shared buffer:")