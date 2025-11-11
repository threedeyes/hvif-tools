#!/usr/bin/env python3
import sys
import os
import subprocess
import tempfile
import shutil
import inkex

class IOMOutput(inkex.OutputExtension):
    def save(self, stream):
        if not shutil.which('svg2iom'):
            raise inkex.AbortExtension(
                "svg2iom utility not found. Please install Icon-O-Matic tools."
            )
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.svg', delete=False) as svg_file:
            svg_file.write(self.document.getroot().tostring().decode('utf-8'))
            svg_file.flush()
            svg_path = svg_file.name
        
        try:
            with tempfile.NamedTemporaryFile(mode='rb', suffix='.iom', delete=False) as iom_file:
                iom_path = iom_file.name
            
            try:
                result = subprocess.run(
                    ['svg2iom', svg_path, iom_path],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
            except subprocess.TimeoutExpired:
                raise inkex.AbortExtension("Icon-O-Matic conversion timeout exceeded (30s)")
            except FileNotFoundError:
                raise inkex.AbortExtension("svg2iom not found in PATH")
            
            if result.returncode != 0:
                raise inkex.AbortExtension(f"Failed to convert to Icon-O-Matic: {result.stderr}")
            
            with open(iom_path, 'rb') as f:
                stream.write(f.read())
            
        finally:
            if os.path.exists(svg_path):
                os.unlink(svg_path)
            if 'iom_path' in locals() and os.path.exists(iom_path):
                os.unlink(iom_path)

if __name__ == '__main__':
    IOMOutput().run()
