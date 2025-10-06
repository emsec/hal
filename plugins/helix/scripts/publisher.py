import sys
import json

try:
    import redis
except ImportError:
    print("Error: Missing Python module. Please install 'redis' before running this script.")
    sys.exit(1)


def main() -> None:
    sequence_number: int = 0
    client: redis.Redis = redis.Redis(host="localhost", port=6379, db=0)

    while True:
        channels: list[str] = input("Enter channels: ").split()
        if not channels:
            break

        command: str = input("Enter command: ")
        identifiers: list[str] = input("Enter identifiers: ").split()

        for channel in channels:
            client.publish(channel, json.dumps({"sequence_number": sequence_number, "command": command,"instances": identifiers}))

        sequence_number += 1
        print()


if __name__ == "__main__":
    main()
