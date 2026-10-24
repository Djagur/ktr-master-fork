#!/usr/bin/env python3

import json
from pathlib import Path

DATASETS = ["dbpedia", "nell"]
INPUT_DIR = Path(".")
OUTPUT_DIR = Path("query")
NUM_QUERIES_PER_RULE = 10
WILDCARD_ID = 10_000_000

TARGET = [
    {
        "HeadID": WILDCARD_ID,
        "HeadKind": True,
        "LinkID": WILDCARD_ID,
        "TailID": WILDCARD_ID,
        "TailKind": True,
    }
]

# Fixed labels and relations required by each rule.
RULE_REQUIREMENTS = {
    0: (2, 1),
    1: (1, 1),
    2: (1, 0),
    3: (3, 2),
    4: (1, 1),
    5: (2, 0),
    6: (1, 0),
}

RULE_DESCRIPTIONS = {
    0: "2 fixed labels, 1 fixed relation",
    1: "1 fixed label, 1 fixed relation; wildcard target",
    2: "1 fixed label; wildcard target and relation",
    3: "3 fixed labels, 2 fixed relations",
    4: "1 fixed label, 1 fixed relation; wildcard targets and second relation",
    5: "2 fixed labels; wildcard source and relations",
    6: "1 fixed label; wildcard targets and relations",
}


def load_name_map(path):
    """Map each exact name to its ID."""
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)

    name_to_id = {}
    for item in data:
        name = item["Name"]
        entity_id = int(item["ID"])

        if name in name_to_id:
            raise ValueError(
                f"Duplicate name {name!r} in {path}. Name lookup would be ambiguous."
            )

        name_to_id[name] = entity_id

    return name_to_id


def lookup_name(prompt, name_to_id):
    """Prompt until an exact name is found."""
    while True:
        name = input(prompt).strip()

        if name in name_to_id:
            return name_to_id[name]

        # Helpful case-insensitive fallback.
        matches = [
            known_name
            for known_name in name_to_id
            if known_name.casefold() == name.casefold()
        ]

        if len(matches) == 1:
            matched_name = matches[0]
            print(
                f"Using exact dataset name: {matched_name!r} "
                f"(ID {name_to_id[matched_name]})"
            )
            return name_to_id[matched_name]

        if matches:
            print("Ambiguous case-insensitive match. Please type the exact name.")
        else:
            print(f"Name not found: {name!r}")
            print("Check spelling and capitalization in the dataset JSON.")


def ask_query(rule, iteration, label_map, link_map):
    """Collect the fixed IDs needed for one rule."""
    label_count, link_count = RULE_REQUIREMENTS[rule]

    print(f"\nQuery p{rule}{iteration}: rho{rule}")
    print(f"Pattern: {RULE_DESCRIPTIONS[rule]}")

    labels = []
    links = []

    for i in range(label_count):
        labels.append(
            lookup_name(
                f"  Fixed label {i + 1} name: ",
                label_map,
            )
        )

    for i in range(link_count):
        links.append(
            lookup_name(
                f"  Fixed relation {i + 1} name: ",
                link_map,
            )
        )

    return labels, links


def make_context(rule, labels, links):
    def fixed_label(label_id):
        return {"EntityID": label_id, "EntityKind": True}

    def wildcard_label():
        return {"EntityID": WILDCARD_ID, "EntityKind": True}

    def edge(head, link_id, tail):
        return {
            "HeadID": head,
            "LinkID": link_id,
            "TailID": tail,
        }

    if rule == 0:
        entities = [fixed_label(labels[0]), fixed_label(labels[1])]
        edges = [edge(0, links[0], 1)]

    elif rule == 1:
        entities = [fixed_label(labels[0]), wildcard_label()]
        edges = [edge(0, links[0], 1)]

    elif rule == 2:
        entities = [fixed_label(labels[0]), wildcard_label()]
        edges = [edge(0, WILDCARD_ID, 1)]

    elif rule == 3:
        entities = [
            fixed_label(labels[0]),
            fixed_label(labels[1]),
            fixed_label(labels[2]),
        ]
        edges = [
            edge(0, links[0], 1),
            edge(0, links[1], 2),
        ]

    elif rule == 4:
        entities = [
            fixed_label(labels[0]),
            wildcard_label(),
            wildcard_label(),
        ]
        edges = [
            edge(0, links[0], 1),
            edge(0, WILDCARD_ID, 2),
        ]

    elif rule == 5:
        entities = [
            wildcard_label(),
            fixed_label(labels[0]),
            fixed_label(labels[1]),
        ]
        edges = [
            edge(0, WILDCARD_ID, 1),
            edge(0, WILDCARD_ID, 2),
        ]

    elif rule == 6:
        entities = [
            fixed_label(labels[0]),
            wildcard_label(),
            wildcard_label(),
        ]
        edges = [
            edge(0, WILDCARD_ID, 1),
            edge(0, WILDCARD_ID, 2),
        ]

    else:
        raise ValueError(f"Unknown rule rho{rule}")

    return [{"Entity": entities, "Link": edges}]


def write_json(path, data):
    with path.open("w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write("\n")


def write_queries_list(dataset_dir):
    """
    Write the complete, ordered list of expected query folder names.
    Existing folders are included whether or not they were generated
    during the current run.
    """
    queries = [
        f"p{rule}{iteration}"
        for rule in range(7)
        for iteration in range(NUM_QUERIES_PER_RULE)
    ]

    write_json(dataset_dir / "queries.json", queries)
    print(f"Updated {dataset_dir / 'queries.json'}")


def generate_dataset(dataset):
    print(f"\n{'=' * 50}\nDataset: {dataset}\n{'=' * 50}")

    label_map = load_name_map(INPUT_DIR / f"dataset/{dataset}/{dataset}_label.json")
    link_map = load_name_map(INPUT_DIR / f"dataset/{dataset}/{dataset}_link.json")

    # Confirm that the wildcard ID is not a real dataset ID.
    if WILDCARD_ID in label_map.values():
        raise ValueError(f"{dataset}: wildcard ID collides with a label ID")
    if WILDCARD_ID in link_map.values():
        raise ValueError(f"{dataset}: wildcard ID collides with a link ID")

    dataset_dir = OUTPUT_DIR / dataset
    dataset_dir.mkdir(parents=True, exist_ok=True)

    generated = 0
    skipped = 0

    for rule in range(7):
        for iteration in range(NUM_QUERIES_PER_RULE):
            folder_name = f"p{rule}{iteration}"
            query_dir = dataset_dir / folder_name

            # Skip existing folders before asking for any input.
            if query_dir.exists():
                print(f"Skipping {query_dir} (folder already exists)")
                skipped += 1
                continue

            labels, links = ask_query(rule, iteration, label_map, link_map)

            query_dir.mkdir(parents=True)

            context = make_context(rule, labels, links)
            write_json(query_dir / "context.json", context)
            write_json(query_dir / "target.json", TARGET)

            print(f"Saved {query_dir}")
            generated += 1

    # Always refresh the complete folder-name list.
    write_queries_list(dataset_dir)

    print(f"\n{dataset}: generated {generated}, skipped {skipped}, expected total 70.")


def main():
    print("Interactive context.json generator")
    print("Existing query folders will be skipped.")
    print("Delete a query folder if you want to recreate it.")
    print("Press Ctrl+C to stop.\n")

    for dataset in DATASETS:
        generate_dataset(dataset)

    print("\nAll datasets processed.")


if __name__ == "__main__":
    main()
