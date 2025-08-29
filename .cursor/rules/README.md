# Browserwind Cursor Rules System

This directory contains Cursor AI coding rules specifically optimized for Browserwind development.

## 📁 Rule Files

### Core Rules (Always Applied)
- **`project-philosophy.mdc`** - Project identity, naming conventions, development principles
- **`ai-assistance.mdc`** - AI behavior guidelines, code generation preferences, error prevention

### Language-Specific Rules
- **`cpp-standards.mdc`** - C++11/14/17 coding standards, memory management, style guidelines
- **`game-development.mdc`** - Game development patterns, ECS architecture, performance optimization
- **`performance.mdc`** - Performance profiling, optimization techniques, budget monitoring

## 🎯 Rule Categories

### 1. **Project Philosophy** (Always Applied)
- Fantasy-themed naming conventions
- System separation patterns
- Development principles and goals

### 2. **C++ Standards** (Applied to *.cpp, *.hpp, *.h files)
- Modern C++ features and idioms
- Memory management (smart pointers, RAII)
- Code style and formatting (Allman bracing, 4-space indentation)
- Exception safety patterns

### 3. **Game Development** (Applied to game files)
- Entity-Component-System (ECS) architecture
- Game loop patterns and fixed timestep
- Rendering optimization (LOD, culling, batching)
- AI and behavior systems

### 4. **Performance** (Applied broadly)
- Profiling and benchmarking
- Memory optimization (object pooling, custom allocators)
- Algorithm optimization (spatial partitioning, caching)
- Performance budgeting and monitoring

### 5. **AI Assistance** (Always Applied)
- Code generation preferences
- Error prevention and detection
- Browserwind-specific intelligence
- Testing and quality assurance

## 🚀 Using the Rules

### Automatic Application
Rules are automatically applied based on:
- **File extensions** (e.g., C++ rules for *.cpp files)
- **Always applied rules** for general guidance
- **Context-aware suggestions** based on code patterns

### Manual Activation
You can reference specific rules in your queries:
- "Apply C++ standards to this function"
- "Use Browserwind naming conventions"
- "Optimize this for performance"

## 📊 Rule Priorities

### STRICT (Must Follow)
- Rule 3.1-3.2: Modern C++ Features & Smart Pointer Usage
- Rule 2.1-2.3: Code Style Consistency
- Rule 1.1: Fantasy-Themed Naming

### RECOMMENDED (Should Follow)
- Rule 4.1: ECS Pattern Adoption
- Rule 4.3: Performance Optimization
- Rule 5.1-5.3: Rendering Standards

### OPTIONAL (Nice to Have)
- Rule 7.2: Advanced AI Systems
- Rule 8.1-8.2: Audio Features
- Rule 10.1-10.2: Testing Infrastructure

## 🎯 Development Workflow

### 1. **New Feature Development**
- AI will suggest fantasy-themed names automatically
- Modern C++ patterns will be used by default
- Performance considerations will be included

### 2. **Code Review**
- Anti-patterns will be flagged automatically
- Memory safety issues will be detected
- Performance bottlenecks will be identified

### 3. **Refactoring**
- Legacy code will be modernized automatically
- Smart pointers will replace raw pointers
- ECS patterns will be suggested for new systems

### 4. **Optimization**
- Performance profiling will be recommended
- Memory optimization techniques will be suggested
- Rendering bottlenecks will be identified

## 🔧 Customization

### Adding New Rules
1. Create new `.mdc` file in this directory
2. Use proper frontmatter metadata
3. Follow Markdown formatting
4. Test with sample code

### Modifying Existing Rules
1. Edit the appropriate `.mdc` file
2. Update frontmatter if changing scope
3. Test changes with existing codebase
4. Update this README if needed

## 📈 Benefits

### For Developers
- **Consistency**: Uniform coding standards across the project
- **Quality**: Automatic detection of common mistakes
- **Performance**: Built-in optimization suggestions
- **Productivity**: Faster development with intelligent assistance

### For the Project
- **Maintainability**: Clear, consistent codebase structure
- **Scalability**: Modular architecture supporting growth
- **Performance**: Optimized systems from the ground up
- **Quality**: Comprehensive testing and error prevention

## 🎮 Browserwind-Specific Intelligence

The rules include deep understanding of:
- **Fantasy RPG Mechanics**: Morrowind-inspired systems
- **Performance Requirements**: 60 FPS target with optimization
- **Architecture Patterns**: ECS, modular systems, modern C++
- **Development Goals**: Production-quality game engine

---

## 📝 Rule Format Reference

```markdown
---
alwaysApply: true/false          # Apply to all requests
description: string             # AI can find rule by description
globs: *.ext,*.ext2            # File patterns to apply rule
---
# Rule content in Markdown format
```

**Example:**
```markdown
---
globs: *.cpp,*.hpp
description: C++ coding standards for Browserwind
---
# C++ Standards
- Use smart pointers
- Modern C++ features
- Exception safety
```

## 🔄 Maintenance

### Regular Updates
- Review rules quarterly for improvements
- Update for new C++ standards
- Add rules for new Browserwind features
- Incorporate team feedback

### Testing
- Test rules with sample code
- Verify AI suggestions are appropriate
- Ensure rules don't conflict
- Validate performance impact

---

*These rules are specifically tailored for Browserwind's fantasy 3D FPS game development using modern C++ and raylib.*
