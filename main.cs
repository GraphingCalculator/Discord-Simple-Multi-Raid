using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;

namespace DiscordSpammer
{
    class Program
    {
        static List<string> tokens = new List<string>();
        static readonly object tokensLock = new object();

        static void ReadTokensFromFile()
        {
            try
            {
                string[] lines = File.ReadAllLines("tokens.txt");
                lock (tokensLock)
                {
                    tokens.AddRange(lines);
                }
            }
            catch (FileNotFoundException)
            {
                File.Create("tokens.txt").Close();
            }
        }

        static void WriteToTokensFile(string token)
        {
            lock (tokensLock)
            {
                File.AppendAllText("tokens.txt", token + Environment.NewLine);
            }
        }

        static string GetRandomToken()
        {
            lock (tokensLock)
            {
                if (tokens.Count == 0)
                    return null;

                int randomIndex = new Random().Next(tokens.Count);
                return tokens[randomIndex];
            }
        }

        static (string, string) Setup()
        {
            Console.WriteLine("Enter your serverID below:");
            string serverId = Console.ReadLine();

            Console.WriteLine("Enter your message below:");
            string msg = Console.ReadLine();

            return (serverId, msg);
        }

        static async Task MainLoopAsync(string serverId, string msg)
        {
            while (true)
            {
                string token = GetRandomToken();
                if (token == null)
                {
                    Console.WriteLine("No tokens available. Exiting...");
                    return;
                }

                string url = $"https://discord.com/api/v8/channels/{serverId}/messages";
                string payload = $"{{\"content\":\"{msg}\"}}";

                using (HttpClient client = new HttpClient())
                {
                    client.DefaultRequestHeaders.Add("Authorization", token);
                    client.DefaultRequestHeaders.Add("Content-Type", "application/json");

                    HttpResponseMessage response = await client.PostAsync(url, new StringContent(payload));
                    Console.WriteLine(response.StatusCode);

                    if (response.StatusCode == HttpStatusCode.Unauthorized)
                    {
                        // Token is invalid remove it from list and try again
                        lock (tokensLock)
                        {
                            tokens.Remove(token);
                        }
                    }
                    else
                    {
                        // Success or other err
                        break;
                    }
                }

                // Sleep for a while before sending the next message
                await Task.Delay(500);
            }
        }

        static async Task Main(string[] args)
        {
            ReadTokensFromFile();

            (string serverId, string msg) = Setup();

            int numThreads = 60;
            List<Task> tasks = new List<Task>();

            for (int i = 0; i < numThreads; i++)
            {
                tasks.Add(MainLoopAsync(serverId, msg));
            }

            await Task.WhenAll(tasks);
        }
    }
}
