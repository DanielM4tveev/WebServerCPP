FROM gcc:latest
WORKDIR /app
COPY WebServer.c .
COPY index.html .
COPY style.css .
RUN gcc WebServer.c -o WebServer
EXPOSE 8088
CMD ["./WebServer"]
