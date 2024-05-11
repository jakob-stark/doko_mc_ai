import random
import logging
import contextlib

import protocol


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
    def __init__(self, args):
        self.args = args

    def run(self):
        with contextlib.ExitStack() as stack:
            # initialize the agents
            team1 = [
                stack.enter_context(protocol.Agent(self.args.agent1, 'agent1')),
                stack.enter_context(protocol.Agent(self.args.agent1, 'agent2')),
            ]
            team2 = [
                stack.enter_context(protocol.Agent(self.args.agent2, 'agent3')),
                stack.enter_context(protocol.Agent(self.args.agent2, 'agent4')),
            ]

            cards = list(generate_cards())
            agents = [*team1, *team2]
            for agent in agents:
                agent.points = 0

            for n in range(self.args.number):
                random.shuffle(cards)
                # random.shuffle(agents)

                for i, agent in enumerate(agents):
                    agent.call(
                        'initialize',
                        computer_player='player{}'.format(i+1),
                        starting_player='player1',
                        cards=[
                            {'suit': suit, 'value': value}
                            for suit, value in cards[12*i:12*(i+1)]
                        ],
                    )
                logger.info('initialization done')

                for agent in agents:
                    agent.call(
                        'start',
                        game='normal',
                        starting_player='player1',
                    )
                logger.info('agents started')

                current_player = 0
                for i in range(48):
                    if i % 4 == 0:
                        logger.info('--- trick {} ---'.format(i // 4))
                    move = agents[current_player].call('get_move')
                    logger.info('player{} plays {}'.format(current_player + 1, move['card']))
                    next_player = [
                        agent.call(
                            'do_move',
                            player='player{}'.format(current_player + 1),
                            move=move
                        ) for agent in agents
                    ]
                    current_player = int(next_player[0][-1:])-1

                for agent in agents:
                    agent.points += agent.call('finish')

            # calculate results
            for team in (team1, team2):
                points = sum(agent.points for agent in team) / 2
                name = team[0].name
                print('team {} makes {} points in {} matches'.format(name, points, self.args.number))

