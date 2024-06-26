import socket 
import threading
import json
import time
import pandas as pd
import numpy as np
import queue
from interbotix_xs_modules.xs_robot.arm import InterbotixManipulatorXS
Rob = InterbotixManipulatorXS("px150", "arm", "gripper")

HEADER = 64
PORT = 8888
# SERVER = "172.24.234.53"
SERVER = "127.0.0.1"
ADDR = (SERVER, PORT)
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "DISCONNECT"
msg_Queue = queue.Queue()
parsed_Queue = queue.Queue()
global total_array
total_array = pd.DataFrame()

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)

def handle_client(conn, addr):
    print(f"[NEW CONNECTION] {addr} connected.")
    connected = True
    while connected:
        # Receive the length of the message
        msg_length = conn.recv(HEADER).decode(FORMAT)
        if msg_length:
            msg_length = int(msg_length)  # Convert the length from string to int

            # Send an acknowledgement
            conn.send("OK".encode(FORMAT))
            # Receive the message
            msg = ''
            bytes_received = 0
            while bytes_received < msg_length:
                chunk = conn.recv(min(msg_length - bytes_received, 2048)).decode(FORMAT)
                bytes_received += len(chunk)
                msg += chunk
            # Parse the message as a JSON array
            msg = json.loads(msg)
            if msg == DISCONNECT_MESSAGE:
                connected = False
            else:
                # Get the received time
                received_time = time.time()
                # Add the received time to the message
                msg.append(received_time)
                # Add the message to the queue
                msg_Queue.put(msg)
                
                if msg_Queue.qsize() > 30:
                    parsed_Queue.get()
    conn.close()

def start():
    server.listen(-1)
    print("[LISTENING] Server is listening on {0}".format(SERVER))
    while True:
        
        conn, addr = server.accept()
        thread_connection = threading.Thread(target=handle_client, args=(conn, addr))
        thread_received = threading.Thread(target = received_data)
        thread_parsed = threading.Thread(target = parsed_data)
        print("[ACTIVE CONNECTIONS] {0}".format(threading.active_count() - 1))
        thread_connection.start()
        thread_received.start()
        thread_parsed.start()

        
def received_data():
    global total_array
    avg_Queue = queue.Queue()  # Queue to compute averages

    while True:
        try:
            # Wait for up to 1 second for a new message to arrive.
            msg = msg_Queue.get(timeout=1)
        except queue.Empty:
            continue

        # Add the new message to the avg queue.
        avg_Queue.put(np.array(msg))

        # Ensure the avg queue only keeps the last 6 messages.
        while avg_Queue.qsize() > 2:
            avg_Queue.get()

        # If there are enough messages in the avg queue, compute the average.
        if avg_Queue.qsize() == 2:
            total_array = pd.DataFrame(list(avg_Queue.queue))
            avg_array = total_array.mean(axis=0)
            avg_list = avg_array.tolist()

            # Store the average in the parsed queue.
            parsed_Queue.put(avg_list)
            if parsed_Queue.qsize() > 4:
                parsed_Queue.get()



    
    
def parsed_data():
    global total_array
    
    # Theoretical mapping
    px150_x = 407
    px150_y = 424
    px150_z = 450
    leap_x = 230.0
    leap_y = 450.0
    leap_z = 230.0
    offset_x = 0.2 # forward, stretch away from the back
    offset_y = 0 # left 0.24
    offset_z = 0.1 # up 0.1, raise above the base
    parsedData = None
    while True:
        # try to execute the code, if error, pass and do function again
        try:
            # takes the first out of the parsed_Queue and sends it to the robot, then deletes the rest of the queue values
            parsedData = parsed_Queue.get()
            
        except IndexError:
            pass
        if parsedData is not None:

            # changes the parsed data into an array
            np.array(parsedData)
            

            hm_x = parsedData[0]/1000.0
            hm_y = -parsedData[2]/1000.0
            hm_z = parsedData[1]/1000.0
            wrist = parsedData[3]



            rb_x = hm_y*(px150_x-offset_x)/leap_z + offset_x
            rb_y = -hm_x*(px150_y-offset_y)/leap_x + offset_y
            rb_z = hm_z*(px150_z-offset_z)/leap_y + offset_z
            #print(f"rb_x: {rb_x}, rb_y: {rb_y}, rb_z: {rb_z}, roll: {wrist}, pitch: {parsedData[4]}, yaw: {parsedData[5]}, grab: {parsedData[6]}")
            # parsedData[2] = LEAP motion x value, parsedData[0] = LEAP motion y value, parsedData[1] = LEAP motion z value, parsedData[4] = roll value of hand, parsedData[5] = grab strength (0 to 1)

            Rob.arm.set_ee_pose_components(x=rb_x, y=rb_y, z=rb_z, roll = wrist , pitch = parsedData[4], yaw = parsedData[5], moving_time=0.5, accel_time=0.25, execute=True, blocking=False)
            grab = parsedData[6]
           # print(f"grab: {grab}")
             #sets the gripper to open or closed
            if grab:
                Rob.gripper.set_pressure(1.0)
                Rob.gripper.grasp(0.1)
            else:
                Rob.gripper.set_pressure(1.0)
                Rob.gripper.release(0.3)
        
            # determines if the robot successfully moved or not
            #successful_move = Rob.arm.get_robot_move()
            

        else:
            traceback.print_exc()
            pass

print("[STARTING] Server is starting...")
start()
