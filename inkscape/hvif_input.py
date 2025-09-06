#!/usr/bin/env python3
import sys
import os
import subprocess
import tempfile
import inkex

class HVIFInput(inkex.InputExtension):
    def load(self, stream):
        with tempfile.NamedTemporaryFile(mode='wb', suffix='.hvif', delete=False) as hvif_file:
            hvif_file.write(stream.read())
            hvif_file.flush()
            hvif_path = hvif_file.name
        
        try:
            with tempfile.NamedTemporaryFile(mode='r', suffix='.svg', delete=False) as svg_file:
                svg_path = svg_file.name
            
            result = subprocess.run(
                ['hvif2svg', hvif_path, svg_path],
                capture_output=True,
                text=True
            )
            
            if result.returncode != 0:
                raise inkex.AbortExtension(f"Failed to convert HVIF file: {result.stderr}")
            
            with open(svg_path, 'rb') as f:
                svg_content = f.read()
            
            return svg_content
            
        finally:
            if os.path.exists(hvif_path):
                os.unlink(hvif_path)
            if 'svg_path' in locals() and os.path.exists(svg_path):
                os.unlink(svg_path)

if __name__ == '__main__':
    HVIFInput().run()
