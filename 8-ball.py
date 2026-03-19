import random

# Expanded lists
binary = ["No", "Yes", "Maybe", "Figure it out yourself.", "Absolutely not", "Depends..."]
who = ["You", "Me", "Tim Arnold III", "Elon Munch", "Albert Larry Gregory XXVI", "Epstien", "Your neighbor", "The cat", "Nobody", "The CIA", "The Magic 8-Ball itself", "A mysterious stranger"]
where = ["Nowhere", "I don't know", "Over there", "Little St. James Island", "A Cardboard box", "The Batcave", "Under your bed", "On the moon", "In your fridge", "The Upside Down", "Somewhere over the rainbow"]
how = ["idk", "By doing it", "figure it out yourself", "idc", "You stinky", "arch btw", "Carefully", "With style", "Without permission", "Magically", "Like a pro"]
why = ["because", "Why not?", "You got a problem with that?", "Because they can", "I don't know.", "Because you smell", "For fun", "To confuse you", "Because chaos"]
what = ["Beer", "Fish", "i don't know", "Something", "Albert", "Lungs", "Oxygen", "Pickles", "Mystery", "Your mom", "Infinity", "Chocolate", "Bananas", "Arch Linux"]
what2 = ["Eating", "Drinking", "Breathing", "Sitting in an electric chair", "Devouring U-235", "Enriching Plutonium-239", "Licking the fuel rods", "Drinking bleach", "Eating sounds", "Sniffing smelling salts", "Electrocuting myself","Eating dirt", "Contemplating existence", "Running away from responsibility", "Tickling unicorns"]
when = [f"{i} minutes" for i in range(1, 61)] + [f"{i} hours" for i in range(1, 25)] + [f"{i} days" for i in range(1, 366)]
how_many = [str(i) for i in range(5000)]  # Numbers 0-4999

# Mapping keywords to your lists
responses = {
    "who": who,
    "where": where,
    "how": how,
    "why": why,
    "is": binary,
    "can": binary,
    "will": binary,
    "do": binary,
    "are": binary,
    "am": binary,
    "when": when,
    "does": binary,
    "did": binary,
    "should": binary
}

print("--- MAGIC 8-BALL IS ACTIVE (Press Ctrl+C to quit) ---")

try:
    while True:
        inp = input("Magic 8-ball, ask whatever the fuck you want: ")
        words = [w.lower().strip("?!.") for w in inp.split()]

        if words:
            firstword = words[0]
            
            # 1. Handle "How many" specifically
            if firstword == "how" and len(words) > 1 and words[1] in ["many", "much"]:
                # This will give you numbers up to 1 quintillion
                print(f"{random.randint(0, 10**18):,}") 

            
            # 2. Handle "What" context (Actions vs Objects)
            elif firstword == "what":
                if any(w in words for w in ["are", "do", "doing", "was", "happening"]):
                    print(random.choice(what2)) # Actions
                else:
                    print(random.choice(what))  # Objects
            
            # 3. Handle everything else in the dictionary
            elif firstword in responses:
                print(random.choice(responses[firstword]))
            
            # 4. Fallback for unrecognized questions
            else:
                print("I don't even know what you're asking.")
        else:
            print("Say something first!")
            
        print("-" * 10)

except KeyboardInterrupt:
    print("GIT OUTTA HERE. SCRAM. GIT. SHOO. SKIDDADLE. GO. DO NOT COLLECT $200, DO NOT PASS GO.")