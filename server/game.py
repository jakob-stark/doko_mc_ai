import random
import logging
import contextlib


logger = logging.getLogger(__name__)


def generate_cards():
    for suit in ('diamond', 'heart', 'spade', 'club'):
        for value in ('nine', 'jack', 'queen', 'king', 'ten', 'ace'):
            yield (suit, value)
            yield (suit, value)


def get_value(card):
    suit, value = card
    return {
        'nine': 0, 'jack': 2, 'queen': 3, 'king': 4, 'ten': 10, 'ace': 11
    }[value]


def get_rank(card):
    base_ranks = {
        'nine': 1, 'jack': 2, 'queen': 3, 'king': 4, 'ten': 5, 'ace': 6,
        'diamond': 1, 'heart': 2, 'spade': 3, 'club': 4
    }
    suit, value = card
    if card == ('heart', 'ten'):
        return 'trump', 300
    if value in ('jack', 'queen'):
        return 'trump', 200 + 10 * base_ranks[value] + base_ranks[suit]
    if suit == 'diamond':
        return 'trump', 100 + base_ranks[value]
    return suit, base_ranks[value]


class Game:
    def __init__(self, agents):
        assert len(agents) == 4
        self.agents = {
            'player{}'.format(i+1): agent for i, agent in enumerate(agents)
        }

    def run(self):
        with contextlib.ExitStack() as stack:
            # deal the cards
            cards = list(generate_cards())
            random.shuffle(cards)

            # initialize the agents
            for i, (player, agent) in enumerate(self.agents.items()):
                stack.enter_context(agent)
                agent.call(
                    'initialize',
                    computer_player=player,
                    starting_player='player1',
                    cards=[
                        {'suit': suit, 'value': value}
                        for suit, value in cards[12*i:12*(i+1)]
                    ],
                )
            logger.info('initialization done')

            for agent in self.agents.values():
                agent.call(
                    'start',
                    game='normal',
                    starting_player='player1',
                )
            logger.info('agents started')

            self.current_player = 'player1'
            for i in range(48):
                if i % 4 == 0:
                    logger.info('--- trick {} ---'.format(i // 4))
                agent = self.agents[self.current_player]
                move = agent.call(
                    'get_move',
                )
                logger.info('{} plays {}'.format(self.current_player, move['card']))
                next_player = [
                    agent.call(
                        'do_move',
                        player=self.current_player,
                        move=move
                    ) for agent in self.agents.values()
                ]
                self.current_player = next_player[0]
