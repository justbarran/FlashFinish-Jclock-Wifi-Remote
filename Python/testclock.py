import requests
import time
import argparse

#python testclock.py --key SetTimeUpOver --value 001234

def send_get_request(base_url, params):
    # Perform a GET request
    response = requests.get(base_url, params=params)
    print("Status Code:", response.status_code)
    #print("Response Text:", response.text)
    # Clear values of parameters after sending request
    clear_params_values(params)
    return response

def clear_params_values(params):
    # Set all parameter values to an empty string or appropriate default
    for key in params.keys():
        params[key] = ''  # or another default if necessary

def update_params(params, updates):
    # Update parameters with new values
    params.update(updates)
    return params

def update_params_key(params, key, value):
    if key in params:
        params[key] = value
    else:
        print(f"Parameter {key} not found in the list.")

def battery_function(arg=None):
    if arg:
        print(f"Running 'battery' function with argument: {arg}")
        # Update parameters for the second request
        updates = {'AutoBright': 'off'}  # Set 'SetTimeUpOver' to '001234'
        update_params(params, updates)
        # Send another GET request with the updated parameters
        print("Sending updated GET request...")
        send_get_request(base_url, params)
        time.sleep(0.1)
        updates = {'SetBrht': arg}  # Set 'SetTimeUpOver' to '001234'
        update_params(params, updates)
        # Send another GET request with the updated parameters
        print("Sending updated GET request...")
        send_get_request(base_url, params)

    else:
        print("Running 'go' function without specific argument.")
    # Additional code for the 'go' command can utilize `arg`

def go_function(arg=None):
    if arg:
        print(f"Running 'go' function with argument: {arg}")
        # Update parameters for the second request
        updates = {'SetTimeUpOver': arg}  # Set 'SetTimeUpOver' to '001234'
        update_params(params, updates)
        # Send another GET request with the updated parameters
        print("Sending updated GET request...")
        send_get_request(base_url, params)

    else:
        print("Running 'go' function without specific argument.")
    # Additional code for the 'go' command can utilize `arg`

def stop_function(arg=None):

    if arg:
        print(f"Running 'stop' function with argument: {arg}")
        # Update parameters for the first request
        updates = {'SetTimeUpOver': arg}  # Set 'SetTimeUpOver' to '001234'
        update_params(params, updates)
        # Send initial command with original parameters
        print("Sending initial GET request...")
        send_get_request(base_url, params)
        # Wait for 500 milliseconds (0.5 seconds)
        time.sleep(0.1)
    else:
        print("Running 'stop' function without specific argument.")

    # Additional code for the 'stop' command can utilize `arg`
    # Update parameters for the second request
    updates = {'PauseClock': 'enable'}  # Set 'SetTimeUpOver' to '001234'
    update_params(params, updates)
    # Send another GET request with the updated parameters
    print("Sending updated GET request...")
    send_get_request(base_url, params)



# Setup argparse
parser = argparse.ArgumentParser(description="Control script behavior with command line arguments.")
parser.add_argument('command', nargs='?', help='Command to execute (go, stop)')
parser.add_argument('argument', nargs='?', help='Optional argument for command')
parser.add_argument('--key', type=str, help='Parameter name to set')
parser.add_argument('--value', type=str, help='Value for the parameter')

args = parser.parse_args()

# Base URL without parameters
base_url = 'http://192.168.1.253/action_page.php'

# Initialize parameters with default values
params = {
    'SetTimeUpOver': '',
    'SetTimeDwnStp': '',
    'SetTimeDwnbUp': '',
    'PauseClock': '',
    'AutoBright': '',
    'SetBrht': '',
    'AutoBattery': '',
    'ShowBattery': '',
    'DiagnosticCommand': ''
}

# Execute command or update parameters
if args.command:
    if args.command == 'go':
        go_function(args.argument)
    elif args.command == 'stop':
        stop_function(args.argument)
    elif args.command == 'battery':
        battery_function(args.argument)
else:
    # Update parameter from command line argument
    if args.key and args.value:
        update_params(params, args.key, args.value)
        print("Sending GET request...")
        send_get_request(base_url, params)


# Update parameter from command line argument
# if args.key and args.value:
#     update_params_key(params, args.key, args.value)



