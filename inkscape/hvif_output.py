#!/usr/bin/env python3
import sys
import os
import subprocess
import tempfile
import shutil
import inkex

class HVIFOutput(inkex.OutputExtension):
    def save(self, stream):
        if not shutil.which('svg2hvif'):
            raise inkex.AbortExtension(
                "svg2hvif utility not found. Please install Haiku icon tools."
            )

        with tempfile.NamedTemporaryFile(mode='w', suffix='.svg', delete=False) as svg_file:
            svg_file.write(self.document.getroot().tostring().decode('utf-8'))
            svg_file.flush()
            svg_path = svg_file.name
        
        try:
            with tempfile.NamedTemporaryFile(mode='rb', suffix='.hvif', delete=False) as hvif_file:
                hvif_path = hvif_file.name
            
            try:
                result = subprocess.run(
                    ['svg2hvif', svg_path, hvif_path],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
            except subprocess.TimeoutExpired:
                raise inkex.AbortExtension("HVIF conversion timeout exceeded (30s)")
            except FileNotFoundError:
                raise inkex.AbortExtension("svg2hvif not found in PATH")
            
            if result.returncode != 0:
                raise inkex.AbortExtension(f"Failed to convert to HVIF: {result.stderr}")
            
            with open(hvif_path, 'rb') as f:
                stream.write(f.read())
            
        finally:
            if os.path.exists(svg_path):
                os.unlink(svg_path)
            if 'hvif_path' in locals() and os.path.exists(hvif_path):
                os.unlink(hvif_path)

if __name__ == '__main__':
    HVIFOutput().run()
