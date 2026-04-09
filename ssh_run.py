import paramiko
import sys

def run_command(host, user, password, command):
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        client.connect(host, username=user, password=password, timeout=10)
        stdin, stdout, stderr = client.exec_command(command)
        out = stdout.read().decode('utf-8')
        err = stderr.read().decode('utf-8')
        if out:
            print(out)
        if err:
            print("STDERR:", err, file=sys.stderr)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
    finally:
        client.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 ssh_run.py <command>")
        sys.exit(1)
    
    command = sys.argv[1]
    run_command('45.32.248.214', 'root', 'At6$t!6]$vV{@sVz', command)
