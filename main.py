import requests, os, json, threading, random
from colorama import *

tokens = []

try:
    with open("tokens.txt", "r") as file:
        for line in file:
            tokens.append(line.strip())
except Exception:
    with open("tokens.txt", "w") as f:
        pass


def setup():
    print("Enter your serverID below")
    serverId = int(input(f"{Fore.LIGHTGREEN_EX}>"))

    print("Enter your message below")
    msg = input(f"{Fore.LIGHTGREEN_EX}>")

    return serverId, msg


serv, ms = setup()


def main():
    # Main
    while True:
        payload = {
            "content": ms
        }
        header = {
            "authorization": random.choice(tokens)
        }

        resp = requests.post(url=f"https://discord.com//api/v8/channels/{serv}/messages", json=payload, headers=header)
        print(resp.status_code)


if __name__ == "__main__":
    threads = []
    for i in range(60):
        t = threading.Thread(target=main)
        t.daemon = True
        threads.append(t)
    for i in range(len(threads)):
        threads[i].start()
    for i in range(len(threads)):
        threads[i].join()
