# ChistaDATA Authentication API Documentation

This repository contains the documentation for the ChistaDATA Authentication APIs.


## Table of Contents



## Introduction

We have a embedded basic authentication system in ChistaDATA. The authentication system is based on the json web token (JWT) .

## API Endpoints

- POST /auth/login
  - Request
    - Body (application/json)
    - Example
  
        ```json
        {
            "username": "admin",
            "password": "admin"
        }
        ```

  - Response
    - Body (application/json)
    - Example
  
        ```json
        {
            "token": "eyJhbGciOi..."
        }
        ```

- POST /auth/logout
  - Request
    - Headers
      - Authorization (Bearer token)
      - Example
  
          ```shell
          Authorization: Bearer eyJhbGciOi...
          ```

  - Response
    - Body (application/json)
    - Example
  
        ```json
        {
            "message": "Logged out"
        }
        ```
