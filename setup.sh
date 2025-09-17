#!/bin/bash

# HFT Trading Application - Quick Build Script
# Professional Intraday Trading System Setup

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ASCII Art Header
echo -e "${CYAN}"
echo "=========================================================="
echo "🚀 HFT TRADING APPLICATION - QUICK SETUP"
echo "   Professional Intraday Trading System"
echo "=========================================================="
echo -e "${NC}"

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    echo -e "${BLUE}📱 Detected OS: Linux${NC}"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
    echo -e "${BLUE}📱 Detected OS: macOS${NC}"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    OS="windows"
    echo -e "${BLUE}📱 Detected OS: Windows${NC}"
else
    echo -e "${RED}❌ Unsupported OS: $OSTYPE${NC}"
    exit 1
fi

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install dependencies
install_dependencies() {
    echo -e "${YELLOW}🔧 Installing dependencies...${NC}"
    
    if [[ "$OS" == "linux" ]]; then
        # Ubuntu/Debian
        if command_exists apt-get; then
            echo -e "${BLUE}📦 Installing packages with apt-get...${NC}"
            sudo apt-get update -qq
            sudo apt-get install -y build-essential cmake \
                libsfml-dev libtgui-dev libcurl4-openssl-dev \
                nlohmann-json3-dev pkg-config git
        # Arch Linux
        elif command_exists pacman; then
            echo -e "${BLUE}📦 Installing packages with pacman...${NC}"
            sudo pacman -S --noconfirm gcc cmake sfml tgui \
                curl nlohmann-json git
        # Red Hat/CentOS/Fedora
        elif command_exists yum || command_exists dnf; then
            echo -e "${YELLOW}⚠️  Red Hat/CentOS detected. You may need to install packages manually.${NC}"
            echo -e "${BLUE}Try: sudo dnf install gcc-c++ cmake SFML-devel libcurl-devel json-devel${NC}"
        else
            echo -e "${RED}❌ Could not detect package manager. Please install dependencies manually.${NC}"
            exit 1
        fi
    elif [[ "$OS" == "macos" ]]; then
        if command_exists brew; then
            echo -e "${BLUE}🍺 Installing packages with Homebrew...${NC}"
            brew install sfml tgui curl nlohmann-json cmake
        else
            echo -e "${RED}❌ Homebrew not found. Please install Homebrew first:${NC}"
            echo -e "${BLUE}/bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"${NC}"
            exit 1
        fi
    elif [[ "$OS" == "windows" ]]; then
        echo -e "${YELLOW}⚠️  Windows detected. Please use MSYS2 or Visual Studio to install:${NC}"
        echo -e "${BLUE}   - SFML 2.5+${NC}"
        echo -e "${BLUE}   - TGUI 0.9+${NC}"
        echo -e "${BLUE}   - libcurl${NC}"
        echo -e "${BLUE}   - nlohmann-json${NC}"
        echo -e "${BLUE}   - CMake 3.16+${NC}"
    fi
    
    echo -e "${GREEN}✅ Dependencies installed successfully!${NC}"
}

# Function to build the project
build_project() {
    echo -e "${YELLOW}🏗️  Building HFT Trading Application...${NC}"
    
    # Create build directory
    if [[ ! -d "build" ]]; then
        mkdir build
        echo -e "${BLUE}📁 Created build directory${NC}"
    fi
    
    cd build
    
    # Configure with CMake
    echo -e "${BLUE}⚙️  Configuring with CMake...${NC}"
    if ! cmake ..; then
        echo -e "${RED}❌ CMake configuration failed. Trying with Makefile...${NC}"
        cd ..
        if [[ -f "Makefile" ]]; then
            make clean
            if make release; then
                echo -e "${GREEN}✅ Built successfully with Makefile!${NC}"
                return 0
            else
                echo -e "${RED}❌ Build failed with both CMake and Makefile${NC}"
                return 1
            fi
        else
            echo -e "${RED}❌ No Makefile found either${NC}"
            return 1
        fi
    fi
    
    # Build
    echo -e "${BLUE}🔨 Compiling source code...${NC}"
    if [[ "$OS" == "linux" ]]; then
        make -j$(nproc)
    elif [[ "$OS" == "macos" ]]; then
        make -j$(sysctl -n hw.ncpu)
    else
        make -j4
    fi
    
    cd ..
    echo -e "${GREEN}✅ Build completed successfully!${NC}"
}

# Function to run the application
run_application() {
    echo -e "${YELLOW}🚀 Starting HFT Trading Application...${NC}"
    
    if [[ -f "build/HFTTradingApp" ]]; then
        echo -e "${GREEN}🎯 Application ready! Starting GUI...${NC}"
        echo -e "${BLUE}📊 Loading live market data...${NC}"
        echo -e "${BLUE}📈 Initializing trading strategies...${NC}"
        echo -e "${CYAN}=========================================================="
        echo "💡 TIP: Use Ctrl+C to stop the application"
        echo "📋 Check the GUI tabs for live data, signals, and history"
        echo "⚠️  DISCLAIMER: For educational purposes only - not financial advice"
        echo "==========================================================${NC}"
        
        # Create necessary directories
        mkdir -p data resources
        
        # Run the application
        ./build/HFTTradingApp
    else
        echo -e "${RED}❌ Executable not found. Build may have failed.${NC}"
        exit 1
    fi
}

# Function to show post-installation info
show_info() {
    echo -e "${CYAN}=========================================================="
    echo "🎉 HFT TRADING APPLICATION SETUP COMPLETE!"
    echo "=========================================================="
    echo -e "${NC}"
    echo -e "${GREEN}✅ Application built and ready to use${NC}"
    echo -e "${BLUE}📱 Executable location: ./build/HFTTradingApp${NC}"
    echo -e "${BLUE}📁 Data directory: ./data/${NC}"
    echo -e "${BLUE}📋 Logs directory: ./logs/${NC}"
    echo ""
    echo -e "${YELLOW}🚀 QUICK START:${NC}"
    echo -e "${BLUE}   ./build/HFTTradingApp          # Run the application${NC}"
    echo -e "${BLUE}   make run                       # Alternative run command${NC}"
    echo ""
    echo -e "${YELLOW}📚 FEATURES:${NC}"
    echo -e "${BLUE}   • Live Nifty 50 data with Yahoo Finance API${NC}"
    echo -e "${BLUE}   • 5+ Professional trading strategies${NC}"
    echo -e "${BLUE}   • Real-time technical analysis${NC}"
    echo -e "${BLUE}   • Market scanner for opportunities${NC}"
    echo -e "${BLUE}   • Trading signal generation${NC}"
    echo -e "${BLUE}   • Performance tracking & history${NC}"
    echo ""
    echo -e "${YELLOW}⚙️  CONFIGURATION:${NC}"
    echo -e "${BLUE}   • Edit config files in ./resources/${NC}"
    echo -e "${BLUE}   • Adjust update intervals and thresholds${NC}"
    echo -e "${BLUE}   • Customize GUI themes and layout${NC}"
    echo ""
    echo -e "${YELLOW}📖 DOCUMENTATION:${NC}"
    echo -e "${BLUE}   • README.md - Complete user guide${NC}"
    echo -e "${BLUE}   • GitHub repository for updates${NC}"
    echo ""
    echo -e "${RED}⚠️  DISCLAIMER:${NC}"
    echo -e "${RED}   This software is for EDUCATIONAL purposes only.${NC}"
    echo -e "${RED}   Not financial advice. Trade at your own risk.${NC}"
    echo ""
    echo -e "${CYAN}=========================================================="
    echo "Happy Trading! 📈🚀"
    echo "==========================================================${NC}"
}

# Main execution
main() {
    case "${1:-all}" in
        "deps")
            install_dependencies
            ;;
        "build")
            build_project
            ;;
        "run")
            run_application
            ;;
        "all")
            echo -e "${YELLOW}🔄 Running complete setup...${NC}"
            install_dependencies
            echo ""
            build_project
            echo ""
            show_info
            echo ""
            echo -e "${YELLOW}🎮 Would you like to run the application now? (y/N)${NC}"
            read -r response
            if [[ "$response" =~ ^[Yy]$ ]]; then
                run_application
            else
                echo -e "${BLUE}👍 Setup complete! Run './build/HFTTradingApp' when ready.${NC}"
            fi
            ;;
        "help"|"--help"|"-h")
            echo -e "${CYAN}HFT Trading Application - Quick Setup Script${NC}"
            echo ""
            echo -e "${YELLOW}Usage:${NC}"
            echo -e "${BLUE}  $0 [command]${NC}"
            echo ""
            echo -e "${YELLOW}Commands:${NC}"
            echo -e "${BLUE}  all     Complete setup (default) - install deps, build, show info${NC}"
            echo -e "${BLUE}  deps    Install dependencies only${NC}"
            echo -e "${BLUE}  build   Build project only${NC}"
            echo -e "${BLUE}  run     Run the application${NC}"
            echo -e "${BLUE}  help    Show this help message${NC}"
            echo ""
            echo -e "${YELLOW}Examples:${NC}"
            echo -e "${BLUE}  $0           # Complete setup${NC}"
            echo -e "${BLUE}  $0 deps      # Install dependencies only${NC}"
            echo -e "${BLUE}  $0 build     # Build only${NC}"
            echo -e "${BLUE}  $0 run       # Run application${NC}"
            ;;
        *)
            echo -e "${RED}❌ Unknown command: $1${NC}"
            echo -e "${BLUE}Run '$0 help' for usage information${NC}"
            exit 1
            ;;
    esac
}

# Check if script is being sourced or executed
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
