#!/usr/bin/env python3
"""
Kconfig Parser for LaminPie
This script parses Kconfig files and generates configuration headers.
"""

import os
import sys
import argparse
import re
from pathlib import Path

class KconfigParser:
    def __init__(self):
        self.configs = {}
        self.menus = []
        self.current_menu = None
        
    def parse_file(self, filepath):
        """Parse a Kconfig file"""
        try:
            # Set current file directory for relative path resolution
            self.current_file_dir = os.path.dirname(os.path.abspath(filepath))
            
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
                self._parse_content(content)
        except Exception as e:
            print(f"Error parsing {filepath}: {e}")
            return False
        return True
    
    def _parse_content(self, content):
        """Parse Kconfig content"""
        lines = content.split('\n')
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            
            # Skip empty lines and comments
            if not line or line.startswith('#'):
                i += 1
                continue
                
            # Parse different Kconfig constructs
            if line.startswith('menu '):
                self._parse_menu(lines, i)
            elif line.startswith('config '):
                i = self._parse_config(lines, i)
            elif line.startswith('menuconfig '):
                i = self._parse_menuconfig(lines, i)
            elif line.startswith('choice '):
                i = self._parse_choice(lines, i)
            elif line.startswith('endchoice'):
                self.current_menu = None
            elif line.startswith('endmenu'):
                self.current_menu = None
            elif line.startswith('rsource '):
                self._parse_rsource(line)
            
            i += 1
    
    def _parse_menu(self, lines, start_idx):
        """Parse menu definition"""
        line = lines[start_idx]
        match = re.match(r'menu\s+"([^"]+)"', line)
        if match:
            menu_name = match.group(1)
            self.menus.append(menu_name)
            self.current_menu = menu_name
    
    def _parse_config(self, lines, start_idx):
        """Parse config definition"""
        line = lines[start_idx]
        match = re.match(r'config\s+(\w+)', line)
        if not match:
            return start_idx + 1
            
        config_name = match.group(1)
        config_info = {
            'name': config_name,
            'type': 'bool',
            'default': None,
            'help': '',
            'menu': self.current_menu
        }
        
        # Parse subsequent lines for this config
        i = start_idx + 1
        while i < len(lines):
            line = lines[i].strip()
            
            if not line or line.startswith('#'):
                i += 1
                continue
                
            if line.startswith(('config ', 'menu ', 'menuconfig ', 'choice ', 'endmenu', 'endchoice')):
                break
                
            # Parse config attributes
            if line.startswith('bool'):
                config_info['type'] = 'bool'
            elif line.startswith('int'):
                config_info['type'] = 'int'
            elif line.startswith('string'):
                config_info['type'] = 'string'
            elif line.startswith('default '):
                default_value = line[8:].strip()
                if default_value in ('y', 'n'):
                    config_info['default'] = default_value == 'y'
                elif default_value.isdigit():
                    config_info['default'] = int(default_value)
                else:
                    config_info['default'] = default_value.strip('"')
            elif line.startswith('help'):
                # Parse help text
                help_lines = []
                i += 1
                while i < len(lines) and lines[i].startswith('\t'):
                    help_lines.append(lines[i].strip())
                    i += 1
                config_info['help'] = ' '.join(help_lines)
                continue
            
            i += 1
        
        self.configs[config_name] = config_info
        return i - 1
    
    def _parse_menuconfig(self, lines, start_idx):
        """Parse menuconfig definition"""
        line = lines[start_idx]
        match = re.match(r'menuconfig\s+(\w+)', line)
        if not match:
            return start_idx + 1
            
        config_name = match.group(1)
        config_info = {
            'name': config_name,
            'type': 'bool',
            'default': True,  # menuconfig defaults to enabled
            'help': '',
            'menu': self.current_menu,
            'is_menuconfig': True
        }
        
        # Parse subsequent lines
        i = start_idx + 1
        while i < len(lines):
            line = lines[i].strip()
            
            if not line or line.startswith('#'):
                i += 1
                continue
                
            if line.startswith(('config ', 'menu ', 'menuconfig ', 'choice ', 'endmenu', 'endchoice')):
                break
                
            if line.startswith('bool'):
                config_info['type'] = 'bool'
            elif line.startswith('default '):
                default_value = line[8:].strip()
                config_info['default'] = default_value == 'y'
            elif line.startswith('help'):
                help_lines = []
                i += 1
                while i < len(lines) and lines[i].startswith('\t'):
                    help_lines.append(lines[i].strip())
                    i += 1
                config_info['help'] = ' '.join(help_lines)
                continue
            
            i += 1
        
        self.configs[config_name] = config_info
        return i - 1
    
    def _parse_choice(self, lines, start_idx):
        """Parse choice definition"""
        line = lines[start_idx]
        match = re.match(r'choice\s+(\w+)', line)
        choice_name = match.group(1) if match else "CHOICE"
        
        # Parse choice options
        i = start_idx + 1
        while i < len(lines):
            line = lines[i].strip()
            
            if not line or line.startswith('#'):
                i += 1
                continue
                
            if line.startswith('endchoice'):
                break
                
            if line.startswith('config '):
                # Parse config within choice
                i = self._parse_config(lines, i)
            elif line.startswith('default '):
                # Parse default choice
                default_value = line[8:].strip()
                # Store choice default
                if hasattr(self, 'choice_defaults'):
                    self.choice_defaults[choice_name] = default_value
                else:
                    self.choice_defaults = {choice_name: default_value}
            
            i += 1
        return i
    
    def _parse_rsource(self, line):
        """Parse rsource directive"""
        match = re.match(r'rsource\s+"([^"]+)"', line)
        if match:
            rsource_path = match.group(1)
            # Resolve relative path from current file directory
            if hasattr(self, 'current_file_dir'):
                full_path = os.path.join(self.current_file_dir, rsource_path)
            else:
                full_path = rsource_path
            
            if os.path.exists(full_path):
                # Store current directory for relative path resolution
                old_dir = getattr(self, 'current_file_dir', None)
                self.current_file_dir = os.path.dirname(full_path)
                self.parse_file(full_path)
                self.current_file_dir = old_dir
    
    def generate_config_header(self, output_file, default_configs=None):
        """Generate configuration header file"""
        if default_configs is None:
            default_configs = {}
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("/*\n")
            f.write(" * Auto-generated configuration header\n")
            f.write(" * Generated by kconfig_parser.py\n")
            f.write(" */\n\n")
            f.write("#ifndef AUTOGEN_CONFIG_H\n")
            f.write("#define AUTOGEN_CONFIG_H\n\n")
            
            # Generate config definitions
            for config_name, config_info in self.configs.items():
                config_value = default_configs.get(config_name, config_info['default'])
                
                if config_info['type'] == 'bool':
                    if config_value:
                        f.write(f"#define CONFIG_{config_name} 1\n")
                    else:
                        f.write(f"#define CONFIG_{config_name} 0\n")
                elif config_info['type'] == 'int':
                    f.write(f"#define CONFIG_{config_name} {config_value}\n")
                elif config_info['type'] == 'string':
                    f.write(f"#define CONFIG_{config_name} \"{config_value}\"\n")
            
            f.write("\n#endif // AUTOGEN_CONFIG_H\n")
    
    def print_summary(self):
        """Print parsing summary"""
        print(f"Parsed {len(self.configs)} configurations:")
        for config_name, config_info in self.configs.items():
            print(f"  {config_name}: {config_info['type']} (default: {config_info['default']})")

def main():
    parser = argparse.ArgumentParser(description='Kconfig Parser for LaminPie')
    parser.add_argument('kconfig_file', help='Path to main Kconfig file')
    parser.add_argument('-o', '--output', help='Output configuration header file')
    parser.add_argument('-d', '--defaults', help='Default configuration file')
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    # Create parser instance
    kconfig_parser = KconfigParser()
    
    # Parse main Kconfig file
    if not kconfig_parser.parse_file(args.kconfig_file):
        sys.exit(1)
    
    # Load default configurations if provided
    default_configs = {}
    if args.defaults and os.path.exists(args.defaults):
        with open(args.defaults, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):
                    if '=' in line:
                        key, value = line.split('=', 1)
                        key = key.strip()
                        value = value.strip()
                        
                        # Convert value based on type
                        if value.lower() in ('y', 'yes', 'true', '1'):
                            default_configs[key] = True
                        elif value.lower() in ('n', 'no', 'false', '0'):
                            default_configs[key] = False
                        elif value.isdigit():
                            default_configs[key] = int(value)
                        else:
                            default_configs[key] = value.strip('"')
    
    # Generate output
    if args.output:
        kconfig_parser.generate_config_header(args.output, default_configs)
        print(f"Generated configuration header: {args.output}")
    
    if args.verbose:
        kconfig_parser.print_summary()

if __name__ == '__main__':
    main()
