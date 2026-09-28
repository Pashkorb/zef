import argparse

import serial


def parse_byte_value(value_text):
    parsed_value = int(value_text, 0)

    if (0 > parsed_value) or (0xFF < parsed_value):
        raise argparse.ArgumentTypeError("value must be between 0 and 255")

    return parsed_value



def create_argument_parser():
    argument_parser = argparse.ArgumentParser()
    argument_parser.add_argument("--port", required=True)
    argument_parser.add_argument("--baud-rate", type=int, default=115200)
    argument_parser.add_argument(
        "--value",
        type=parse_byte_value,
        default=0,
    )
    argument_parser.add_argument("--timeout", type=float, default=1.0)

    return argument_parser



def exchange_byte(port_name, baud_rate, request_value, timeout_seconds):
    with serial.Serial(
        port=port_name,
        baudrate=baud_rate,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=timeout_seconds,
        write_timeout=timeout_seconds,
    ) as serial_port:
        serial_port.reset_input_buffer()
        serial_port.write(bytes([request_value]))
        serial_port.flush()
        response_data = serial_port.read(1)

    if (1 != len(response_data)):
        raise TimeoutError("The device did not return one byte before timeout")

    return response_data[0]



def run_client():
    arguments = create_argument_parser().parse_args()
    response_value = exchange_byte(
        arguments.port,
        arguments.baud_rate,
        arguments.value,
        arguments.timeout,
    )
    expected_value = (arguments.value + 1) & 0xFF

    print(f"sent=0x{arguments.value:02X}")
    print(f"received=0x{response_value:02X}")
    print(f"expected=0x{expected_value:02X}")

    return int(response_value != expected_value)



if "__main__" == __name__:
    raise SystemExit(run_client())
