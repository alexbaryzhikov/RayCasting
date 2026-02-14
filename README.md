# RayCasting Engine

A high-performance raycasting engine built for macOS using [Metal](https://developer.apple.com/metal/) and C++.

## Demo

https://github.com/user-attachments/assets/6e9131e6-c97d-424c-a76f-fb2403c41842

## Description

This project implements a classic raycasting rendering technique, similar to Wolfenstein 3D, but accelerated using Apple's Metal API. It features textured rendering, dynamic lighting, shadows, fog, and a map system. The core logic is written in C++ with Objective-C handling the macOS application layer and Metal integration.

## Requirements

-   macOS with Metal support.
-   Xcode.

## Controls

| Keys          | Actions                 |
| ------------- | ------------------------|
| W, S, A, D    | movement                |
| Arrow keys    | movement, map scroll    |
| N             | walls collision         |
| M             | map                     |
| Shift + M     | minimap                 |
| F             | map follow mode         |
| G             | map player presentation |
| +, -          | map zoom                |
| 0             | reset map zoom          |
| Esc           | release mouse           |


## Roadmap

- Save and load map
- Game menu: generate, save and load map
- Toolbar for building different kinds of walls
- Torches as light sources
