import asyncio
import ssl
import argparse


async def handle_client(reader, writer):
    addr = writer.get_extra_info('peername')
    try:
        while True:
            data = await reader.read(4096)
            if not data:
                break
            # Simple echo server for validating TLS framing and round trips
            writer.write(data)
            await writer.drain()
    finally:
        writer.close()
        await writer.wait_closed()


async def main(host: str, port: int, certfile: str = None, keyfile: str = None, no_tls: bool = False):
    if no_tls:
        server = await asyncio.start_server(handle_client, host, port)
        addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
        print(f"Serving plain TCP echo on {addrs}")
        async with server:
            await server.serve_forever()
    else:
        if not certfile or not keyfile:
            raise ValueError("cert and key required unless --no-tls is used")
        ssl_ctx = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        ssl_ctx.load_cert_chain(certfile, keyfile)
        server = await asyncio.start_server(handle_client, host, port, ssl=ssl_ctx)
        addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
        print(f"Serving TLS echo on {addrs}")
        async with server:
            await server.serve_forever()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', default=9000, type=int)
    parser.add_argument('--cert')
    parser.add_argument('--key')
    parser.add_argument('--no-tls', action='store_true', help='Run plain TCP (no TLS)')
    args = parser.parse_args()
    asyncio.run(main(args.host, args.port, args.cert, args.key, args.no_tls))
