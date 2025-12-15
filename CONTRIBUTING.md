# Contributing to TMCxx

Thank you for your interest in contributing to TMCxx!

## Development Setup

```bash
# Clone the repository
git clone https://github.com/REDE-ARGE/TMCxx.git
cd TMCxx

# Configure with tests enabled
cmake -B build -DTMCXX_BUILD_TESTS=ON

# Build
cmake --build build

# Run tests
./build/tests/tmcxx_tests
```

## Code Style

- Follow the existing code style
- Use `.clang-format` for formatting
- All public APIs must have Doxygen documentation

## Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run tests and ensure they pass
5. Commit your changes (`git commit -m 'feat: add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Commit Message Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/):

- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation only
- `refactor:` - Code refactoring
- `test:` - Adding or updating tests
- `chore:` - Maintenance tasks

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
