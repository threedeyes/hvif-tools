#!/usr/bin/env python3
import sys
import os
import subprocess
import tempfile
import shutil
import inkex

class IOMInput(inkex.InputExtension):
    def load(self, stream):
        if not shutil.which('icon2icon'):
            raise inkex.AbortExtension(
                "icon2icon utility not found. Please install HVIF-Tools."
            )
        
        with tempfile.NamedTemporaryFile(mode='wb', suffix='.iom', delete=False) as iom_file:
            iom_file.write(stream.read())
            iom_file.flush()
            iom_path = iom_file.name
        
        try:
            with tempfile.NamedTemporaryFile(mode='r', suffix='.svg', delete=False) as svg_file:
                svg_path = svg_file.name
            
            try:
                result = subprocess.run(
                    ['icon2icon', iom_path, svg_path],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
            except subprocess.TimeoutExpired:
                raise inkex.AbortExtension("Icon-O-Matic conversion timeout exceeded (30s)")
            except FileNotFoundError:
                raise inkex.AbortExtension("icon2icon not found in PATH")
            
            if result.returncode != 0:
                raise inkex.AbortExtension(f"Failed to convert Icon-O-Matic file: {result.stderr}")
            
            with open(svg_path, 'rb') as f:
                svg_content = f.read()
            
            return svg_content
            
        finally:
            if os.path.exists(iom_path):
                os.unlink(iom_path)
            if 'svg_path' in locals() and os.path.exists(svg_path):
                os.unlink(svg_path)

if __name__ == '__main__':
    IOMInput().run()
