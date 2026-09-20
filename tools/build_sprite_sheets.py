from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
INPUT_DIR = ROOT / "assets" / "sprites" / "keyed"
OUTPUT_DIR = ROOT / "assets" / "sprites"

CELL_SIZE = 640
ANCHOR_X = 320
BASELINE_Y = 560

# Maximum visible bounds inside a 640x640 cell. One scale is used for every
# frame of a sheet, so motion remains coherent within each animation.
SHEETS = {
    "cat_walk": {"frames": 8, "duration_ms": 90, "max_w": 540, "max_h": 420},
    "cat_lie_down": {"frames": 12, "duration_ms": 80, "max_w": 540, "max_h": 420},
    "cat_sleep": {"frames": 6, "duration_ms": 350, "max_w": 470, "max_h": 270},
    "cat_poke": {"frames": 6, "duration_ms": 70, "max_w": 500, "max_h": 320},
    "cat_hiss": {"frames": 10, "duration_ms": 90, "max_w": 540, "max_h": 450},
    "cat_settle": {"frames": 6, "duration_ms": 100, "max_w": 540, "max_h": 420},
    "cat_wake_yawn": {"frames": 12, "duration_ms": 150, "max_w": 540, "max_h": 430},
    "cat_turn": {"frames": 4, "duration_ms": 100, "max_w": 540, "max_h": 430},
}


def alpha_bbox(image: Image.Image) -> tuple[int, int, int, int]:
    bbox = image.getchannel("A").getbbox()
    if bbox is None:
        raise ValueError("Frame contains no opaque pixels")
    return bbox


def strip_key_fringe(image: Image.Image) -> Image.Image:
    """Remove green-dominant matte pixels and keep pixel-art alpha hard."""
    cleaned = image.copy()
    pixels = cleaned.load()
    for y in range(cleaned.height):
        for x in range(cleaned.width):
            red, green, blue, alpha = pixels[x, y]
            if alpha == 0:
                continue
            if green > 90 and green > red + 18 and green > blue + 18:
                pixels[x, y] = (red, green, blue, 0)
            else:
                pixels[x, y] = (red, green, blue, 255 if alpha >= 96 else 0)
    return cleaned


def split_source(source: Image.Image, count: int) -> list[Image.Image]:
    """Split on the actual transparent gaps instead of assumed equal cells."""
    alpha = source.getchannel("A")
    occupied = []
    for x in range(source.width):
        _, maximum = alpha.crop((x, 0, x + 1, source.height)).getextrema()
        occupied.append(maximum >= 32)

    runs: list[tuple[int, int]] = []
    start: int | None = None
    for x, has_pixels in enumerate(occupied):
        if has_pixels and start is None:
            start = x
        elif not has_pixels and start is not None:
            if x - start >= 8:
                runs.append((start, x))
            start = None
    if start is not None and source.width - start >= 8:
        runs.append((start, source.width))

    if len(runs) != count:
        raise ValueError(
            f"Detected {len(runs)} character regions, expected {count}: {runs}"
        )

    return [source.crop((left, 0, right, source.height)) for left, right in runs]


def normalize_sheet(name: str, spec: dict[str, int]) -> Image.Image:
    source = Image.open(INPUT_DIR / f"{name}.png").convert("RGBA")
    source_count = 11 if name == "cat_lie_down" else spec["frames"]
    source_frames = split_source(source, source_count)
    boxes = [alpha_bbox(frame) for frame in source_frames]

    maximum_width = max(right - left for left, _, right, _ in boxes)
    maximum_height = max(bottom - top for _, top, _, bottom in boxes)
    scale = min(spec["max_w"] / maximum_width, spec["max_h"] / maximum_height)

    result = Image.new("RGBA", (CELL_SIZE * source_count, CELL_SIZE), (0, 0, 0, 0))

    for index, (frame, bbox) in enumerate(zip(source_frames, boxes, strict=True)):
        subject = strip_key_fringe(frame.crop(bbox))
        subject = subject.crop(alpha_bbox(subject))
        target_width = max(1, round(subject.width * scale))
        target_height = max(1, round(subject.height * scale))
        subject = subject.resize((target_width, target_height), Image.Resampling.NEAREST)

        x = index * CELL_SIZE + ANCHOR_X - target_width // 2
        y = BASELINE_Y - target_height
        result.alpha_composite(subject, (x, y))

    if name == "cat_lie_down":
        # The panoramic generator repeatedly produced 11 poses. A separately
        # generated, genuinely distinct transition is inserted after source
        # frame 7 instead of duplicating a frame.
        missing = Image.open(INPUT_DIR / "cat_lie_down_missing.png").convert("RGBA")
        missing = strip_key_fringe(missing.crop(alpha_bbox(missing)))
        missing = missing.crop(alpha_bbox(missing))
        missing_scale = min(470 / missing.width, 260 / missing.height)
        missing = missing.resize(
            (
                max(1, round(missing.width * missing_scale)),
                max(1, round(missing.height * missing_scale)),
            ),
            Image.Resampling.NEAREST,
        )
        missing_cell = Image.new("RGBA", (CELL_SIZE, CELL_SIZE), (0, 0, 0, 0))
        missing_cell.alpha_composite(
            missing,
            (ANCHOR_X - missing.width // 2, BASELINE_Y - missing.height),
        )

        expanded = Image.new(
            "RGBA", (CELL_SIZE * spec["frames"], CELL_SIZE), (0, 0, 0, 0)
        )
        insertion_index = 7
        for source_index in range(source_count):
            target_index = source_index if source_index < insertion_index else source_index + 1
            expanded.alpha_composite(
                get_cell(result, source_index), (target_index * CELL_SIZE, 0)
            )
        expanded.alpha_composite(missing_cell, (insertion_index * CELL_SIZE, 0))
        result = expanded

    return result


def get_cell(sheet: Image.Image, index: int) -> Image.Image:
    left = index * CELL_SIZE
    return sheet.crop((left, 0, left + CELL_SIZE, CELL_SIZE))


def put_cell(sheet: Image.Image, index: int, cell: Image.Image) -> None:
    left = index * CELL_SIZE
    sheet.paste(cell, (left, 0))


def resize_cell_to_width(cell: Image.Image, target_width: int) -> Image.Image:
    subject = cell.crop(alpha_bbox(cell))
    scale = target_width / subject.width
    subject = subject.resize(
        (target_width, max(1, round(subject.height * scale))),
        Image.Resampling.NEAREST,
    )
    result = Image.new("RGBA", (CELL_SIZE, CELL_SIZE), (0, 0, 0, 0))
    result.alpha_composite(
        subject,
        (ANCHOR_X - subject.width // 2, BASELINE_Y - subject.height),
    )
    return result


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    built = {name: normalize_sheet(name, spec) for name, spec in SHEETS.items()}

    # Enforce the pixel-identical joins required by the animation state machine.
    sleep_start = get_cell(built["cat_sleep"], 0)
    walk_start = get_cell(built["cat_walk"], 0)

    put_cell(built["cat_lie_down"], 0, walk_start)
    put_cell(built["cat_lie_down"], 11, sleep_start)
    put_cell(built["cat_poke"], 0, sleep_start)
    put_cell(built["cat_poke"], 5, sleep_start)
    put_cell(built["cat_hiss"], 0, sleep_start)
    put_cell(built["cat_settle"], 0, get_cell(built["cat_hiss"], 9))
    put_cell(built["cat_settle"], 5, sleep_start)
    put_cell(built["cat_wake_yawn"], 0, sleep_start)
    put_cell(built["cat_wake_yawn"], 11, walk_start)
    put_cell(built["cat_turn"], 0, get_cell(built["cat_wake_yawn"], 11))
    put_cell(
        built["cat_turn"],
        2,
        get_cell(built["cat_turn"], 1).transpose(Image.Transpose.FLIP_LEFT_RIGHT),
    )
    put_cell(built["cat_turn"], 3, walk_start.transpose(Image.Transpose.FLIP_LEFT_RIGHT))

    for index, target_width in enumerate((330, 350, 370, 380), start=1):
        put_cell(
            built["cat_settle"],
            index,
            resize_cell_to_width(get_cell(built["cat_settle"], index), target_width),
        )

    manifest: dict[str, object] = {
        "frame_width": CELL_SIZE,
        "frame_height": CELL_SIZE,
        "anchor_x": ANCHOR_X,
        "baseline_y": BASELINE_Y,
        "sheets": {},
    }

    for name, spec in SHEETS.items():
        sheet = built[name]
        expected_size = (CELL_SIZE * spec["frames"], CELL_SIZE)
        if sheet.size != expected_size:
            raise ValueError(f"{name}: got {sheet.size}, expected {expected_size}")

        frame_bounds = []
        for index in range(spec["frames"]):
            bbox = alpha_bbox(get_cell(sheet, index))
            frame_bounds.append(list(bbox))

        output_path = OUTPUT_DIR / f"{name}.png"
        sheet.save(output_path, format="PNG", optimize=True)
        manifest["sheets"][name] = {
            "file": output_path.name,
            "frames": spec["frames"],
            "duration_ms": spec["duration_ms"],
            "loop": name in {"cat_walk", "cat_sleep"},
            "pixel_bounds": frame_bounds,
        }

    (OUTPUT_DIR / "sprites.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
