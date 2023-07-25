package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"sync"
	"time"
	"net/http"
	"math/rand"
)

var tokens []string
var tokensMutex sync.Mutex

func readTokensFromFile() {
	file, err := os.Open("tokens.txt")
	if err != nil {
		file, _ = os.Create("tokens.txt")
		return
	}
	defer file.Close()

	content, err := ioutil.ReadAll(file)
	if err != nil {
		return
	}

	tokens = strings.Split(string(content), "\n")
}

func writeToTokensFile(token string) {
	tokensMutex.Lock()
	defer tokensMutex.Unlock()

	file, err := os.OpenFile("tokens.txt", os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		return
	}
	defer file.Close()

	file.WriteString(token + "\n")
}

func getRandomToken() string {
	tokensMutex.Lock()
	defer tokensMutex.Unlock()

	if len(tokens) == 0 {
		return ""
	}

	randomIndex := rand.Intn(len(tokens))
	return tokens[randomIndex]
}

func setup() (string, string) {
	var serverID, message string

	fmt.Println("Enter your serverID below:")
	fmt.Scan(&serverID)

	fmt.Println("Enter your message below:")
	fmt.Scan(&message)

	return serverID, message
}

func mainLoop(serverID, message string) {
	for {
		token := getRandomToken()
		if token == "" {
			fmt.Println("No tokens available. Exiting...")
			return
		}

		url := fmt.Sprintf("https://discord.com/api/v8/channels/%s/messages", serverID)
		payload := fmt.Sprintf(`{"content":"%s"}`, message)
		client := &http.Client{}
		req, err := http.NewRequest("POST", url, strings.NewReader(payload))
		if err != nil {
			fmt.Println(err)
			return
		}
		req.Header.Set("Authorization", token)
		req.Header.Set("Content-Type", "application/json")

		resp, err := client.Do(req)
		if err != nil {
			fmt.Println(err)
			return
		}
		defer resp.Body.Close()

		fmt.Println(resp.Status)

		if resp.StatusCode == 401 {
			// Token is invalid remove it from list and try again
			tokensMutex.Lock()
			for i, t := range tokens {
				if t == token {
					tokens = append(tokens[:i], tokens[i+1:]...)
					break
				}
			}
			tokensMutex.Unlock()
		} else {
			// Success or other error (most of time all other errors are invalid token)
			break
		}

		// Sleep for a while before sending the next message can change if you want
		time.Sleep(500 * time.Millisecond)
	}
}

func main() {
	readTokensFromFile()

	serverID, message := setup()

	numThreads := 60
	var wg sync.WaitGroup
	for i := 0; i < numThreads; i++ {
		wg.Add(1)
		go func() {
			mainLoop(serverID, message)
			wg.Done()
		}()
	}
	wg.Wait()
}
